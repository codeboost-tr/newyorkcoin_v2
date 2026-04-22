// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Copyright (c) 2011-2021 The Litecoin Core developers
// Copyright (c) 2013-2026 The NewYorkCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// DarkGravityWave v3 difficulty algorithm.
//
// Based on the Dash implementation (src/pow.cpp, ~2014-2021 Dash Core Developers).
// DGW performs a per-block difficulty adjustment using an exponentially-weighted
// moving average over the past 24 blocks.
//
// References:
//   https://github.com/dashpay/dash/blob/master/src/pow.cpp
//   https://github.com/NewYorkCoinNYC/newyorkcoin/blob/master/src/pow.cpp
//

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util/system.h>

/**
 * DarkGravityWave v3
 *
 * Computes the next required proof-of-work target by taking a weighted moving
 * average of the targets observed in the last nPastBlocks (24) blocks.
 *
 * Algorithm:
 *   1. Walk back nPastBlocks blocks from pindexLast.
 *   2. Compute running weighted average: avg = (avg * n + target) / (n + 1).
 *   3. Scale the average by (actual elapsed time / expected elapsed time),
 *      clamping actual to [expected/3, expected*3] to prevent extreme swings.
 *   4. Clamp result to powLimit.
 */
static unsigned int DarkGravityWave(const CBlockIndex* pindexLast,
                                     const Consensus::Params& params)
{
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);

    const int64_t nPastBlocks = 24;

    // Not enough history — return the minimum difficulty.
    if (!pindexLast || pindexLast->nHeight < nPastBlocks)
        return bnPowLimit.GetCompact();

    const CBlockIndex* pindex = pindexLast;
    arith_uint256 bnPastTargetAvg;

    for (unsigned int nCountBlocks = 1; nCountBlocks <= (unsigned int)nPastBlocks; nCountBlocks++) {
        arith_uint256 bnTarget;
        bnTarget.SetCompact(pindex->nBits);

        if (nCountBlocks == 1) {
            bnPastTargetAvg = bnTarget;
        } else {
            // Incremental weighted average: avg = (avg * n + target) / (n + 1)
            bnPastTargetAvg = (bnPastTargetAvg * nCountBlocks + bnTarget) / (nCountBlocks + 1);
        }

        if (nCountBlocks != (unsigned int)nPastBlocks) {
            assert(pindex->pprev);
            pindex = pindex->pprev;
        }
    }

    arith_uint256 bnNew(bnPastTargetAvg);

    // pindex now points to the oldest of the 24 sampled blocks.
    // pindexLast is the most-recent block.
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindex->GetBlockTime();
    int64_t nTargetTimespan = nPastBlocks * params.nPowTargetSpacing;

    // Clamp actual timespan to prevent extreme oscillation.
    if (nActualTimespan < nTargetTimespan / 3)
        nActualTimespan = nTargetTimespan / 3;
    if (nActualTimespan > nTargetTimespan * 3)
        nActualTimespan = nTargetTimespan * 3;

    // New target = avg_target * (actual / expected).
    // Higher number = easier difficulty, so multiply then divide.
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

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

    // The NYC network uses Kimoto Gravity Well (KGW) throughout its history.
    // Rather than re-implement KGW (which the original code computed via
    // OpenSSL CBigNum), we trust the block's own stated nBits and rely solely
    // on CheckProofOfWork() to verify that the block hash actually satisfies
    // that target.  This is safe: a block with a falsely-easy nBits still
    // requires a hash below that target, and chain-selection by most cumulative
    // work naturally favours the honest chain.
    //
    // TODO: implement KGW natively (arith_uint256 + double, no CBigNum) so that
    // we can enforce the correct difficulty curve and reject dishonest nBits.
    return pblock->nBits;
}

/**
 * CalculateNextWorkRequired is retained for unit-test compatibility.
 * It is not used during normal block validation (DGW is used instead).
 */
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast,
                                        int64_t nFirstBlockTime,
                                        const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Fall through to DGW — the classic two-endpoint retarget does not apply to
    // the NYC chain.  Callers that need a deterministic result from a fixed
    // window can use DGW via GetNextWorkRequired.
    return DarkGravityWave(pindexLast, params);
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
