// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 Dogecoin Developers
// Copyright (c) 2014 Daniel Kraft
// Copyright (c) 2024 NewYorkCoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <auxpow.h>

#include <crypto/common.h>
#include <hash.h>
#include <script/script.h>
#include <uint256.h>
#include <util/system.h>

namespace {

/**
 * Compute a merkle root given a leaf hash, a branch of sibling hashes, and
 * the index of the leaf in the original tx list.  Bit i of nIndex controls
 * which side to pair on at tree level i (0 = right sibling, 1 = left sibling).
 */
static uint256 ComputeAuxMerkleRootFromBranch(const uint256& leaf,
                                              const std::vector<uint256>& branch,
                                              int nIndex)
{
    uint256 hash = leaf;
    for (const uint256& sibling : branch) {
        if (nIndex & 1)
            hash = Hash(sibling.begin(), sibling.end(),
                        hash.begin(), hash.end());
        else
            hash = Hash(hash.begin(), hash.end(),
                        sibling.begin(), sibling.end());
        nIndex >>= 1;
    }
    return hash;
}

} // anonymous namespace

bool CAuxPow::Check(const uint256& hashAuxBlock, int nChainId,
                    const Consensus::Params& params) const
{
    // Merged-mining header magic bytes: 0xfa 0xbe 0x6d 0x6d
    static const unsigned char MERGED_MINING_HEADER[] = {0xfa, 0xbe, 0x6d, 0x6d};

    // (1) The coinbase tx must be the first transaction in the parent block.
    if (nIndex != 0)
        return error("AuxPow is not a coinbase");

    // (2) Verify the coinbase merkle branch proves the coinbase tx is in the
    //     parent block (it correctly chains up to parentBlock.hashMerkleRoot).
    if (ComputeAuxMerkleRootFromBranch(coinbaseTx->GetHash(),
                                       vMerkleBranch, nIndex)
            != parentBlock.hashMerkleRoot)
        return error("AuxPow merkle root incorrect");

    // (3) One-hop attack prevention: the parent block must not itself be an
    //     AuxPoW block for the same chain.  This prevents using the same
    //     parent block to satisfy two NYC blocks in the same chain.
    if (parentBlock.IsAuxpow() && parentBlock.GetChainId() == nChainId)
        return error("AuxPow parent has our chain ID");

    // (4) Locate the merged-mining commitment in the coinbase scriptSig.
    //     Required format after the 4-byte magic:
    //       chain_root (32 bytes, little-endian)
    //       nSize      (uint32 LE, must equal 1 << vChainMerkleBranch.size())
    //       nNonce     (uint32 LE, must equal nChainId % nSize)
    const CScript& script = coinbaseTx->vin[0].scriptSig;

    // Find the LAST occurrence of the magic bytes to prevent padding attacks.
    CScript::const_iterator pcMagic = script.end();
    for (CScript::const_iterator it = script.begin();
         it + 4 <= script.end(); ++it) {
        if (memcmp(&it[0], MERGED_MINING_HEADER, 4) == 0)
            pcMagic = it;
    }

    if (pcMagic == script.end())
        return error("AuxPow missing merged-mining header in coinbase");

    uint256 nRootHash;
    unsigned int nSize;
    unsigned int nNonce;

    {
        CScript::const_iterator pos = pcMagic + 4;
        if (script.end() - pos < 32 + 4 + 4)
            return error("AuxPow coinbase too short after merged-mining header");

        // Chain-tree root stored in natural (little-endian) byte order.
        memcpy(nRootHash.begin(), &pos[0], 32);
        pos += 32;

        nSize  = ReadLE32(&pos[0]);  pos += 4;
        nNonce = ReadLE32(&pos[0]);
    }

    // (5) The chain hash merkle tree size must equal 2^(branch height).
    const unsigned int nTreeSize = 1u << vChainMerkleBranch.size();
    if (nSize != nTreeSize)
        return error("AuxPow chain merkle size mismatch (nSize=%u, expected %u)",
                     nSize, nTreeSize);

    // (6) Anti-hop attack: the slot in the chain merkle tree used by NYC must
    //     be deterministically derived from the chain ID.  This ensures a
    //     given parent block can only solve one chain per tree position.
    const unsigned int nExpectedSlot =
        static_cast<unsigned int>(nChainId) % nSize;
    if (nNonce != nExpectedSlot)
        return error("AuxPow chain nonce mismatch "
                     "(nNonce=%u, expected %u for chainId=%d, nSize=%u)",
                     nNonce, nExpectedSlot, nChainId, nSize);

    // (7) The nChainIndex stored in this CAuxPow must match the coinbase nNonce.
    if (static_cast<unsigned int>(nChainIndex) != nNonce)
        return error("AuxPow chain index mismatch "
                     "(nChainIndex=%d, nNonce=%u)", nChainIndex, nNonce);

    // (8) Verify the chain merkle branch: applying it to hashAuxBlock at
    //     position nChainIndex must reproduce the chain root in the coinbase.
    if (ComputeAuxMerkleRootFromBranch(hashAuxBlock,
                                       vChainMerkleBranch, nChainIndex)
            != nRootHash)
        return error("AuxPow chain merkle root mismatch");

    return true;
}
