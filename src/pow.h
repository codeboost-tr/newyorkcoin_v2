// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Copyright (c) 2011-2021 The Litecoin Core developers
// Copyright (c) 2013-2026 The NewYorkCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POW_H
#define BITCOIN_POW_H

#include <consensus/params.h>

#include <stdint.h>

class CBlockHeader;
class CBlockIndex;
class uint256;

/** Return the next required proof-of-work target using DigiShield (NYC v1.14.x). */
unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast,
                                  const CBlockHeader* pblock,
                                  const Consensus::Params&);

/**
 * Classic two-endpoint retarget calculation.
 * Retained for unit-test compatibility; NYC uses DGW in production.
 */
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast,
                                        int64_t nFirstBlockTime,
                                        const Consensus::Params&);

/** Check whether a block hash satisfies the proof-of-work requirement. */
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params&);

#endif // BITCOIN_POW_H
