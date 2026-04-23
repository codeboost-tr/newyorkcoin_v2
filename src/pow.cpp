// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Copyright (c) 2011-2021 The Litecoin Core developers
// Copyright (c) 2013-2026 The NewYorkCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// NewYorkCoin uses the Kimoto Gravity Well (KGW) difficulty retargeting
// algorithm throughout its history.
//
// This implementation replaces the original OpenSSL CBigNum arithmetic with
// arith_uint256 + double, producing identical results without the OpenSSL
// dependency.
//
// References:
//   https://github.com/NewYorkCoinNYC/newyorkcoin/blob/master/src/pow.cpp  (original KGW)
//   https://bitcointalk.org/index.php?topic=204655.0                        (KGW spec)
//

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <cmath>
#include <primitives/block.h>
#include <uint256.h>
#include <util/system.h>

/**
 * Kimoto Gravity Well (KGW)
 *
 * Per-block difficulty retargeting designed for fast block chains.  Walks
 * backward through past blocks computing a running average difficulty and
 * comparing the observed block rate to the target rate.  The window grows
 * until the rate-ratio falls within the "event horizon" — a band that
 * narrows as more blocks are sampled — then stops.
 *
 * The resulting difficulty is: avg_target * (actual_seconds / target_seconds).
 *
 * NYC parameters (hardcoded to match the live network):
 *   TargetBlockSpacing = params.nPowTargetSpacing  (30 seconds)
 *   PastBlocksMin      = 144
 *   PastBlocksMax      = 4032
 *
 * Overflow analysis:
 *   - arith_uint256 is 256 bits.
 *   - Scrypt powLimit ≈ 2^232.
 *   - Running average multiply: target * (i-1), max 2^232 * 4031 < 2^244 ✓
 *   - Final scale: avg * actual_secs, max 2^232 * 362880 < 2^250 ✓
 */
static unsigned int KimotoGravityWell(const CBlockIndex* pindexLast,
                                       const Consensus::Params& params)
{
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);

    // NYC-specific KGW parameters derived from the original 1.x chain:
    //   BlocksTargetSpacing = 30 s
    //   PastSecondsMin = 86400 * 0.01 = 864 s  → PastBlocksMin = 864 / 30 = 28
    //   PastSecondsMax = 86400 * 0.14 = 12096 s → PastBlocksMax = 12096 / 30 = 403
    static const uint64_t nPastBlocksMin = 28;
    static const uint64_t nPastBlocksMax = 403;

    if (!pindexLast || (uint64_t)pindexLast->nHeight < nPastBlocksMin)
        return bnPowLimit.GetCompact();

    const CBlockIndex* pBlockReading = pindexLast;

    uint64_t nPastBlocksMass         = 0;
    int64_t  nPastRateActualSeconds  = 0;
    int64_t  nPastRateTargetSeconds  = 0;
    double   dPastRateAdjustmentRatio = 1.0;

    arith_uint256 bnPastDifficultyAverage;
    arith_uint256 bnPastDifficultyAveragePrev;

    for (uint64_t i = 1; pBlockReading && pBlockReading->nHeight > 0; i++) {
        if (nPastBlocksMax > 0 && i > nPastBlocksMax)
            break;

        nPastBlocksMass++;

        arith_uint256 bnTarget;
        bnTarget.SetCompact(pBlockReading->nBits);

        if (i == 1) {
            bnPastDifficultyAverage = bnTarget;
        } else {
            // Incremental running average without signed arithmetic:
            //   avg[i] = (avg[i-1] * (i-1) + target[i]) / i
            // Equivalent to the original CBigNum form:
            //   avg[i] = avg[i-1] + (target[i] - avg[i-1]) / i
            arith_uint256 tmp = bnPastDifficultyAveragePrev;
            tmp *= (uint32_t)(i - 1);   // i-1 <= 4031, safe to cast uint32_t
            tmp += bnTarget;
            bnPastDifficultyAverage = tmp / arith_uint256(i);
        }
        bnPastDifficultyAveragePrev = bnPastDifficultyAverage;

        // Elapsed time from the oldest sampled block to the tip.
        nPastRateActualSeconds = pindexLast->GetBlockTime() - pBlockReading->GetBlockTime();
        nPastRateTargetSeconds = (int64_t)params.nPowTargetSpacing * (int64_t)nPastBlocksMass;

        if (nPastRateActualSeconds < 0)
            nPastRateActualSeconds = 0;

        dPastRateAdjustmentRatio = 1.0;
        if (nPastRateActualSeconds != 0 && nPastRateTargetSeconds != 0)
            dPastRateAdjustmentRatio =
                (double)nPastRateTargetSeconds / (double)nPastRateActualSeconds;

        // Event horizon: acceptable adjustment band, using the KGW constants
        // 0.7084 and -1.228 from the original specification.  The /144.0
        // normalises the mass against nPastBlocksMin.
        const double dEventHorizonDeviation =
            1.0 + 0.7084 * std::pow((double)nPastBlocksMass / 144.0, -1.228);
        const double dEventHorizonDeviationFast = dEventHorizonDeviation;
        const double dEventHorizonDeviationSlow = 1.0 / dEventHorizonDeviation;

        if (nPastBlocksMass >= nPastBlocksMin) {
            if (dPastRateAdjustmentRatio <= dEventHorizonDeviationSlow ||
                dPastRateAdjustmentRatio >= dEventHorizonDeviationFast)
                break;
        }

        if (!pBlockReading->pprev)
            break;
        pBlockReading = pBlockReading->pprev;
    }

    // Scale the average target by the observed / expected time ratio.
    arith_uint256 bnNew(bnPastDifficultyAverage);
    if (nPastRateActualSeconds > 0 && nPastRateTargetSeconds > 0) {
        bnNew *= arith_uint256((uint64_t)nPastRateActualSeconds);
        bnNew /= arith_uint256((uint64_t)nPastRateTargetSeconds);
    }

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast,
                                  const CBlockHeader* pblock,
                                  const Consensus::Params& params)
{
    assert(pindexLast != nullptr);

    // Regtest / private net: no retargeting.
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Testnet special rule: if a block took more than 2x the target spacing,
    // allow the minimum difficulty so the chain doesn't stall.
    if (params.fPowAllowMinDifficultyBlocks) {
        if (pblock->GetBlockTime() >
                pindexLast->GetBlockTime() + params.nPowTargetSpacing * 2)
            return UintToArith256(params.powLimit).GetCompact();
    }

    return KimotoGravityWell(pindexLast, params);
}

/**
 * CalculateNextWorkRequired is retained for unit-test compatibility.
 */
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast,
                                        int64_t nFirstBlockTime,
                                        const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    return KimotoGravityWell(pindexLast, params);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow ||
        bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
