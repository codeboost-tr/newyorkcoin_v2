// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Copyright (c) 2011-2021 The Litecoin Core developers
// Copyright (c) 2013-2026 The NewYorkCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// DigiShield difficulty algorithm (as used by the NewYorkCoin v1.14.x network).
//
// The live NYC chain above block 4,800,000 uses a DigiShield retarget that fires
// every DifficultyAdjustmentInterval() = nPowTargetTimespan / nPowTargetSpacing
// = 60 / 30 = 2 blocks.  On odd-height blocks the previous difficulty is
// carried forward unchanged; on even-height blocks a new target is computed.
//
// The amplitude filter damps the adjustment to 1/8 of the observed deviation
// from the target timespan, then clamps to [75 %, 150 %] of the target.
//
// Reference implementation:
//   https://github.com/NewYorkCoinNYC/newyorkcoin/blob/1.14.3-Test/src/pow.cpp
//   https://github.com/NewYorkCoinNYC/newyorkcoin/blob/1.14.3-Test/src/newyorkcoin.cpp
//

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util/system.h>

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast,
                                  const CBlockHeader* pblock,
                                  const Consensus::Params& params)
{
    assert(pindexLast != nullptr);

    const unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Regtest / private net: no retargeting.
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    const int64_t nInterval = params.DifficultyAdjustmentInterval();

    // Off a retarget boundary: carry forward the last block's difficulty.
    // On testnet, allow min difficulty if the block took more than 20× target.
    if ((pindexLast->nHeight + 1) % nInterval != 0) {
        if (params.fPowAllowMinDifficultyBlocks) {
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 20)
                return nProofOfWorkLimit;
            // Return the last non-min-difficulty block.
            const CBlockIndex* pindex = pindexLast;
            while (pindex->pprev &&
                   pindex->nHeight % nInterval != 0 &&
                   pindex->nBits == nProofOfWorkLimit)
                pindex = pindex->pprev;
            return pindex->nBits;
        }
        return pindexLast->nBits;
    }

    // On a retarget boundary: compute new difficulty via DigiShield.
    // Go back by the full interval (except for the very first retarget window).
    int64_t blockstogoback = nInterval;
    if ((pindexLast->nHeight + 1) == nInterval)
        blockstogoback = nInterval - 1;

    int nHeightFirst = pindexLast->nHeight - (int)blockstogoback;
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

/**
 * CalculateNextWorkRequired — DigiShield retarget.
 *
 * Applies the amplitude-filtered DigiShield adjustment used by the NYC v1.14.x
 * network for all blocks above height 4,800,000.
 *
 *   nModulatedTimespan = retarget + (actual - retarget) / 8
 *   clamped to [retarget * 0.75, retarget * 1.50]
 *
 * With nPowTargetTimespan = 60 s:
 *   nMinTimespan = 45 s, nMaxTimespan = 90 s.
 */
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast,
                                        int64_t nFirstBlockTime,
                                        const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    const int64_t retargetTimespan = params.nPowTargetTimespan;
    const int64_t nActualTimespan  = pindexLast->GetBlockTime() - nFirstBlockTime;

    // DigiShield amplitude filter: damp the adjustment to 1/8 of the deviation.
    int64_t nModulatedTimespan = retargetTimespan + (nActualTimespan - retargetTimespan) / 8;

    // Clamp: ±25 % / +50 % of target timespan.
    const int64_t nMinTimespan = retargetTimespan - (retargetTimespan / 4); // 75 %
    const int64_t nMaxTimespan = retargetTimespan + (retargetTimespan / 2); // 150 %
    if (nModulatedTimespan < nMinTimespan) nModulatedTimespan = nMinTimespan;
    if (nModulatedTimespan > nMaxTimespan) nModulatedTimespan = nMaxTimespan;

    // new_target = old_target * modulated / retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nModulatedTimespan;
    bnNew /= retargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
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
