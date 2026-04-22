// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 Dogecoin Developers
// Copyright (c) 2014 Daniel Kraft
// Copyright (c) 2024 NewYorkCoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_AUXPOW_H
#define BITCOIN_AUXPOW_H

#include <consensus/params.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>

#include <memory>
#include <vector>

/**
 * Data for the merge-mining auxpow.  This structure is included in an NYC
 * block when it has been mined via merged mining on a parent blockchain.
 * It proves that the parent block's coinbase commits to this NYC block hash
 * and that the parent block meets NYC's PoW target.
 *
 * Wire format emitted by the original NewYorkCoin daemon
 * (based on the Dogecoin-era CAuxPow : CMerkleTx inheritance):
 *   coinbaseTx           (CTransaction)
 *   hashBlock            (uint256  – parent block hash, legacy CMerkleTx field)
 *   vMerkleBranch        (vector<uint256>  – coinbase merkle branch)
 *   nIndex               (int32             – coinbase position, always 0)
 *   vChainMerkleBranch   (vector<uint256>  – aux-block merkle branch)
 *   nChainIndex          (int32)
 *   parentBlock          (CBlockHeader 80 bytes)
 *
 * Merged-mining commitment in coinbase scriptSig:
 *   0xfabe6d6d  (4-byte magic)
 *   chainRoot   (32 bytes - chain merkle root in REVERSED byte order)
 *   nSize       (uint32 LE - must equal 2^vChainMerkleBranch.size())
 *   nNonce      (uint32 LE - arbitrary; determines slot via GetExpectedIndex)
 */
class CAuxPow
{
public:
    /** The coinbase transaction of the parent block (first tx, nIndex == 0). */
    CTransactionRef coinbaseTx;

    /** The parent block header.  Its PoW hash must satisfy NYC's target. */
    CBlockHeader parentBlock;

    /** Hash of the parent block.  Redundant with parentBlock.GetHash() but
     *  present in the wire format for historical compatibility with the
     *  original Dogecoin-era CMerkleTx serialisation. */
    uint256 hashBlock;

    /** Merkle branch linking coinbaseTx hash to parentBlock.hashMerkleRoot. */
    std::vector<uint256> vMerkleBranch;

    /** Index of the coinbase in the parent block tx list (always 0). */
    int nIndex;

    /** Merkle branch for the NYC block hash within the chain hash merkle tree.
     *  Empty when single-chain merge mining (nSize == 1). */
    std::vector<uint256> vChainMerkleBranch;

    /** Index of the NYC block hash in the chain hash merkle tree.
     *  Must equal GetExpectedIndex(nNonce, nChainId, vChainMerkleBranch.size()). */
    int nChainIndex;

    CAuxPow() : nIndex(0), nChainIndex(0) {}

    SERIALIZE_METHODS(CAuxPow, obj)
    {
        READWRITE(obj.coinbaseTx);
        // hashBlock is the parent-block hash in the legacy CMerkleTx wire
        // format; read/write it for round-trip compatibility with bootstrap
        // data produced by the original NYC daemon.
        READWRITE(obj.hashBlock);
        READWRITE(obj.vMerkleBranch);
        READWRITE(obj.nIndex);
        READWRITE(obj.vChainMerkleBranch);
        READWRITE(obj.nChainIndex);
        READWRITE(obj.parentBlock);
    }

    /**
     * Validate the auxpow proof.
     * @param hashAuxBlock  The hash of the NYC block containing this auxpow.
     * @param nChainId      NYC's chain ID (must match what's in the coinbase).
     * @param params        Consensus parameters.
     * @return True if the proof is valid.
     */
    bool Check(const uint256& hashAuxBlock, int nChainId,
               const Consensus::Params& params) const;

    /** Get the parent block's PoW hash (used to verify against NYC's target). */
    uint256 GetParentBlockPoWHash() const
    {
        return parentBlock.GetPoWHash();
    }

    /**
     * Calculate the expected index in the chain hash merkle tree.
     * Uses the same LCG pseudo-random formula as the original Namecoin /
     * Dogecoin / NYC merged-mining specification.
     * @param nNonce    The nNonce value from the coinbase commitment.
     * @param nChainId  The chain ID.
     * @param h         The chain merkle branch height (log2 of tree size).
     * @return The expected nChainIndex for a valid auxpow.
     */
    static int GetExpectedIndex(unsigned int nNonce, int nChainId, unsigned int h);
};

#endif // BITCOIN_AUXPOW_H
