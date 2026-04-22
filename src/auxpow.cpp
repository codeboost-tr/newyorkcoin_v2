// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 Dogecoin Developers
// Copyright (c) 2014 Daniel Kraft
// Copyright (c) 2024 NewYorkCoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <auxpow.h>

#include <algorithm>
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
            hash = Hash(sibling, hash);
        else
            hash = Hash(hash, sibling);
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

    // Sanity: prevent resource exhaustion from a maliciously long branch.
    if (vChainMerkleBranch.size() > 30)
        return error("AuxPow chain merkle branch too long");

    // (2) Verify the coinbase merkle branch proves the coinbase tx is in the
    //     parent block (it correctly chains up to parentBlock.hashMerkleRoot).
    if (ComputeAuxMerkleRootFromBranch(coinbaseTx->GetHash(),
                                       vMerkleBranch, nIndex)
            != parentBlock.hashMerkleRoot)
        return error("AuxPow merkle root incorrect");

    // (3) One-hop attack prevention: the parent block must not itself be an
    //     AuxPoW block for the same chain.
    if (parentBlock.IsAuxpow() && parentBlock.GetChainId() == nChainId)
        return error("AuxPow parent has our chain ID");

    // (4) Compute the expected chain merkle root from the branch and look for
    //     it in the coinbase scriptSig.  Per the merged-mining spec (and the
    //     original NYC/Dogecoin implementation) the root hash is stored in the
    //     coinbase in REVERSED byte order (big-endian), so we reverse before
    //     searching.
    const uint256 computedRootHash =
        ComputeAuxMerkleRootFromBranch(hashAuxBlock, vChainMerkleBranch, nChainIndex);

    std::vector<unsigned char> vchRootHash(computedRootHash.begin(),
                                           computedRootHash.end());
    std::reverse(vchRootHash.begin(), vchRootHash.end());

    const CScript& script = coinbaseTx->vin[0].scriptSig;

    // Find the chain merkle root (reversed bytes) inside the coinbase.
    CScript::const_iterator pc =
        std::search(script.begin(), script.end(),
                    vchRootHash.begin(), vchRootHash.end());

    if (pc == script.end())
        return error("AuxPow chain merkle root mismatch");

    // (5) Check the placement of the merged-mining header.
    CScript::const_iterator pcHead =
        std::search(script.begin(), script.end(),
                    MERGED_MINING_HEADER, MERGED_MINING_HEADER + 4);

    if (pcHead != script.end()) {
        // Modern format: exactly one header, immediately before the root hash.
        if (script.end() !=
                std::search(pcHead + 1, script.end(),
                            MERGED_MINING_HEADER, MERGED_MINING_HEADER + 4))
            return error("AuxPow multiple merged mining headers in coinbase");
        if (pcHead + 4 != pc)
            return error("AuxPow merged mining header not immediately before chain merkle root");
    } else {
        // Backward compatibility: no header present.  The root must appear
        // within the first 20 bytes of the coinbase (8-12 bytes cover the
        // extraNonce and nBits in very early NYC blocks).
        if (pc - script.begin() > 20)
            return error("AuxPow chain merkle root must start in first 20 bytes of coinbase");
    }

    // (6) Read nSize and nNonce from the 8 bytes immediately after the root.
    pc += vchRootHash.size();
    if (script.end() - pc < 8)
        return error("AuxPow missing chain merkle tree size and nonce in coinbase");

    unsigned int nSize;
    unsigned int nNonce;
    memcpy(&nSize,  &pc[0], 4);
    memcpy(&nNonce, &pc[4], 4);

    // (7) The chain hash merkle tree size must equal 2^(branch height).
    const unsigned int merkleHeight = vChainMerkleBranch.size();
    if (nSize != (1u << merkleHeight))
        return error("AuxPow chain merkle size mismatch (nSize=%u, expected %u)",
                     nSize, 1u << merkleHeight);

    // (8) Verify nChainIndex is the deterministic slot for our chain ID in
    //     the chain merkle tree.  Uses the same LCG as the original NYC and
    //     Dogecoin merged-mining specification.
    if (nChainIndex != GetExpectedIndex(nNonce, nChainId, merkleHeight))
        return error("AuxPow wrong index");

    return true;
}

// static
int CAuxPow::GetExpectedIndex(unsigned int nNonce, int nChainId, unsigned int h)
{
    // Pseudo-random but deterministic slot in the chain merkle tree for a
    // given nNonce / nChainId / tree-height combination.  Prevents the same
    // parent block from solving two different chain IDs at the same position.
    // Algorithm taken verbatim from the original Namecoin / Dogecoin / NYC
    // merged-mining specification.
    unsigned int rand = nNonce;
    rand = rand * 1103515245 + 12345;
    rand += static_cast<unsigned int>(nChainId);
    rand = rand * 1103515245 + 12345;
    return static_cast<int>(rand % (1u << h));
}
