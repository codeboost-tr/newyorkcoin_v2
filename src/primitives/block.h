// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>
#include <mweb/mweb_models.h>

#include <memory>

// Forward declaration — full definition is in auxpow.h (included at bottom of
// this file after CBlockHeader and CBlock are fully defined).
class CAuxPow;

//
// AuxPoW version-field constants.
//
// nVersion layout for an AuxPoW-mined block:
//   bits  0-15 : base version (e.g. 1)
//   bit      8 : BLOCK_VERSION_AUXPOW flag — set when block carries auxpow
//   bits 16-31 : chain ID   (NYC uses 56, stored via nVersion >> 16)
//
// Example NYC AuxPoW block: nVersion = (56 << 16) | 0x100 | 1 = 0x00380101
//
static const int32_t BLOCK_VERSION_AUXPOW      = (1 << 8);   // 0x100
static const int32_t BLOCK_VERSION_CHAIN_ID    = 56;          // NYC chain ID
static const int32_t BLOCK_VERSION_CHAIN_START = (1 << 16);   // 0x10000

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    CBlockHeader()
    {
        SetNull();
    }

    SERIALIZE_METHODS(CBlockHeader, obj) { READWRITE(obj.nVersion, obj.hashPrevBlock, obj.hashMerkleRoot, obj.nTime, obj.nBits, obj.nNonce); }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    uint256 GetPoWHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    /** Return true if this block was mined via merged mining (AuxPoW). */
    bool IsAuxpow() const
    {
        return (nVersion & BLOCK_VERSION_AUXPOW) != 0;
    }

    /** Return the chain ID encoded in nVersion (bits 16-31). */
    int32_t GetChainId() const
    {
        return nVersion >> 16;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // memory only
    mutable bool fChecked;

    MWEB::Block mweb_block;

    // AuxPoW data — present and non-null only when IsAuxpow() is true.
    // The SERIALIZE_METHODS below conditionally includes it, so the wire
    // format is: [CBlockHeader][CAuxPow?][vtx][mweb_block?]
    std::shared_ptr<CAuxPow> auxpow;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *(static_cast<CBlockHeader*>(this)) = header;
    }

    SERIALIZE_METHODS(CBlock, obj)
    {
        READWRITEAS(CBlockHeader, obj);
        if (obj.IsAuxpow()) {
            SER_READ(obj.auxpow, obj = std::make_shared<CAuxPow>());
            READWRITE(*obj.auxpow);
        } else {
            SER_READ(obj.auxpow, obj.reset());
        }
        READWRITE(obj.vtx);
        if (!(s.GetVersion() & SERIALIZE_NO_MWEB)) {
            if (obj.vtx.size() >= 2 && obj.vtx.back()->IsHogEx()) {
                READWRITE(obj.mweb_block);
            }
        }
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
        mweb_block.SetNull();
        auxpow.reset();
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        return block;
    }

    std::string ToString() const;

    // Returns the hogex (integrating) transaction, if it exists.
    CTransactionRef GetHogEx() const noexcept;
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    explicit CBlockLocator(const std::vector<uint256>& vHaveIn) : vHave(vHaveIn) {}

    SERIALIZE_METHODS(CBlockLocator, obj)
    {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(obj.vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

// Include the full CAuxPow definition after CBlockHeader and CBlock are
// defined.  This breaks the circular-include cycle:  auxpow.h includes
// block.h (for CBlockHeader), and block.h includes auxpow.h here (for the
// SERIALIZE_METHODS / READWRITE(*obj.auxpow) instantiation).
// The include guards on both files prevent infinite recursion.
#include <auxpow.h>

#endif // BITCOIN_PRIMITIVES_BLOCK_H
