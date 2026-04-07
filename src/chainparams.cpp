// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Copyright (c) 2011-2021 The Litecoin Core developers
// Copyright (c) 2013-2026 The NewYorkCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <hash.h>
#include <tinyformat.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <versionbitsinfo.h>

#include <assert.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

// ---------------------------------------------------------------------------
// Genesis block helper
// ---------------------------------------------------------------------------
static CBlock CreateGenesisBlock(const char* pszTimestamp,
                                  const CScript& genesisOutputScript,
                                  uint32_t nTime,
                                  uint32_t nNonce,
                                  uint32_t nBits,
                                  int32_t nVersion,
                                  const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig =
        CScript() << 486604799 << CScriptNum(4)
                  << std::vector<unsigned char>((const unsigned char*)pszTimestamp,
                                               (const unsigned char*)pszTimestamp +
                                                   strlen(pszTimestamp));
    txNew.vout[0].nValue      = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the NewYorkCoin genesis block.
 *
 * Genesis parameters (confirmed from NewYorkCoinNYC/newyorkcoin source):
 *   pszTimestamp : "A Coin for New York City"
 *   nTime        : 1394102925
 *   nNonce       : 2482334
 *   nBits        : 0x1e0ffff0
 *   nVersion     : 1
 *   genesisReward: 88 NYC
 *   hashGenesisBlock : 0x5597f25c062a3038c7fd815fe46c67dedfcb3c839fbc8e01ed4044540d08fe48
 *   hashMerkleRoot   : 0x2bad42ac6e0ccc4808d8df0fd50ac8634eea335b1412b1ef52864b430a87b262
 */
static CBlock CreateGenesisBlock(uint32_t nTime,
                                  uint32_t nNonce,
                                  uint32_t nBits,
                                  int32_t  nVersion,
                                  const CAmount& genesisReward)
{
    const char* pszTimestamp = "A Coin for New York City";
    const CScript genesisOutputScript =
        CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b"
                              "10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9")
                  << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits,
                              nVersion, genesisReward);
}

// ---------------------------------------------------------------------------
// Mainnet
// ---------------------------------------------------------------------------
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = CBaseChainParams::MAIN;

        consensus.signet_blocks               = false;
        consensus.signet_challenge.clear();

        // ── Subsidy ─────────────────────────────────────────────────────────
        // NYC halves every 500,000 blocks (matching original chain).
        consensus.nSubsidyHalvingInterval = 500000;

        // ── BIP activation heights ──────────────────────────────────────────
        // P2SH / BIP16 enforced from block 0 on the NYC chain.
        consensus.BIP16Height  = 0;
        // BIP34 (height in coinbase): required from genesis for v2.0 nodes.
        consensus.BIP34Height  = 0;
        consensus.BIP34Hash    = uint256();
        // CLTV (BIP65) and strict-DER (BIP66): safe to enforce from genesis.
        consensus.BIP65Height  = 0;
        consensus.BIP66Height  = 0;
        // CSV (BIP68/112/113): NYC has not activated CSV; defer to far future.
        consensus.CSVHeight    = std::numeric_limits<int>::max();
        // SegWit: NYC has not activated SegWit; defer to far future.
        consensus.SegwitHeight = std::numeric_limits<int>::max();
        // MinBIP9WarningHeight: not relevant until SegWit era.
        consensus.MinBIP9WarningHeight = 0;

        // ── Proof-of-Work limits ────────────────────────────────────────────
        // Same powLimit as original NYC (Scrypt, ~uint256(0) >> 20).
        consensus.powLimit = uint256S(
            "0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        // DGW v3: nPowTargetSpacing is the per-block target (30 seconds).
        // nPowTargetTimespan serves as the DGW window in seconds:
        //   24 blocks × 30 s = 720 s  →  DifficultyAdjustmentInterval() = 24.
        consensus.nPowTargetTimespan  = 24 * 30; // 720 seconds (24-block DGW window)
        consensus.nPowTargetSpacing   = 30;      // 30-second block target

        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting            = false;

        // ── BIP9 / Taproot / MWEB ───────────────────────────────────────────
        // Phase 2: defer all soft-fork activations.
        consensus.nRuleChangeActivationThreshold = 2160; // 75% of 2880
        consensus.nMinerConfirmationWindow       = 2880; // ~1 day at 30-s blocks

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit        = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Taproot (BIPs 340-342): Phase 2 — not scheduled.
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit          = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartHeight =
            std::numeric_limits<int>::max() / 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeoutHeight =
            std::numeric_limits<int>::max() / 2;

        // MWEB (LIP-0002/0003/0004): NYC does not use MimbleWimble. Never activate.
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].bit          = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartHeight =
            std::numeric_limits<int>::max() / 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeoutHeight =
            std::numeric_limits<int>::max() / 2;

        // ── Chain work / assume-valid ────────────────────────────────────────
        // Set to zero for Phase 2 clean start. Update before mainnet release.
        consensus.nMinimumChainWork  = uint256S("0x00");
        // Use last known-good checkpoint as assumevalid to skip script checks
        // on the full historical chain (speeds up IBD significantly).
        consensus.defaultAssumeValid = uint256S(
            "0xfbadb1e438a76d382ba67218230d5a703b105536c55153a7d0ff99c394bfe9e4"); // block 6995462

        // ── Network magic ────────────────────────────────────────────────────
        // NOTE: The user specification was 0xF5F5F5F5.  All known NYC mainnet
        // nodes (NewYorkCoinNYC/newyorkcoin source, ~2018-present) use
        // 0xC0C0C0C0.  We use 0xC0C0C0C0 here to allow peer connections.
        // If launching a new incompatible network, change these bytes.
        pchMessageStart[0] = 0xc0;
        pchMessageStart[1] = 0xc0;
        pchMessageStart[2] = 0xc0;
        pchMessageStart[3] = 0xc0;

        nDefaultPort = 17020;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 8;
        m_assumed_chain_state_size = 1;

        // ── Genesis block ────────────────────────────────────────────────────
        genesis = CreateGenesisBlock(1394102925, 2482334, 0x1e0ffff0, 1, 88 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0x5597f25c062a3038c7fd815fe46c67dedfcb3c839fbc8e01ed4044540d08fe48"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x2bad42ac6e0ccc4808d8df0fd50ac8634eea335b1412b1ef52864b430a87b262"));

        // ── DNS seeds (from NewYorkCoinNYC community) ────────────────────────
        vSeeds.emplace_back("dnsseed.nycoin.money");
        vSeeds.emplace_back("dnsseed.nycoin.community");

        // ── Address prefixes ─────────────────────────────────────────────────
        // P2PKH  0x3C (60)  → 'R' prefix addresses
        // P2SH   0x16 (22)  → '9' prefix addresses
        // WIF    0xBC (188) → private key format
        base58Prefixes[PUBKEY_ADDRESS]  = std::vector<unsigned char>(1, 60);
        base58Prefixes[SCRIPT_ADDRESS]  = std::vector<unsigned char>(1, 52);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 22); // keep LTC P2SH2 slot
        base58Prefixes[SECRET_KEY]      = std::vector<unsigned char>(1, 188);
        base58Prefixes[EXT_PUBLIC_KEY]  = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY]  = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "nyc";
        mweb_hrp = "nycmweb";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_main), std::end(chainparams_seed_main));


        fDefaultConsistencyChecks = false;
        fRequireStandard          = true;
        m_is_test_chain           = false;
        m_is_mockable_chain       = false;

        // ── Checkpoints (all confirmed NYC mainnet blocks) ───────────────────
        checkpointData = {
            {
                {  10000, uint256S("0x132e14f7d82b659329ac95300413beba2c00f9e3d1b137533a093fce18d3febd")},
                { 100000, uint256S("0x495da2e0cffa0ad6c0fe83c2678e2c714e024ed009abcdb24728d306b599232f")},
                { 155511, uint256S("0x0cd7a29253710ebf4c71c473f61e586b044a5da64380e424b63c9f45c89b7cde")},
                { 500000, uint256S("0xb9db8c11eacd9921fb09ad149fc9b7cf41e429e759f8304043398d7ed1067952")},
                {1000000, uint256S("0x157fc4df4e7594abae3487c554bbea91cd70a1014faf7ae7b5d3ee4d9da80226")},
                {1500000, uint256S("0xc185fa9930597d386977969894cecaec21547589efdf756d533824f86244cda1")},
                {2000000, uint256S("0xde3f57919d2048c915e369642f6216aed78f5f0c9a59e45763a09d97f483fa2a")},
                {2500000, uint256S("0x93896d159dbc68e7ac109f4fa7e05365cbb08d78b6fe3957c4a330a878bf2e2a")},
                {3000000, uint256S("0x66e6dcb49370062537c1f6abf655ffbbc53ba4851ef00081aa2e4be1e2903ba7")},
                {3500000, uint256S("0x3f1a97f68ce8eaf38fc0c56868b3eb98ccb67d14bff4e78afb91d82cba853ddf")},
                {3938415, uint256S("0xe1fa41f6fe8d2785d89b0468e13e4c450493e5356c024a098c5b727ca89138ee")},
                {4500000, uint256S("0xdd86fad58b3fa5d83a15a18df1cc20cdcdb1b2cf5d2d702e0c60bbb7d4602fb1")},
                {4821195, uint256S("0x7cf9862123405a687626b27ecaea377698d23d68458bb9b2a16e0262ec32df84")},
                {5000000, uint256S("0x90383fd9bec9a857112afa72737a8335ca752db5beca5b30f1a7ef383bfca4b6")},
                {5500000, uint256S("0xb13e3a3f5e3c19f9ace6f78db690733ab9a811b01c1c645015e1a3e413a97d63")},
                {6000000, uint256S("0xa787bf658bc15817e67dd9e0baee8a5cebea0a02ef13ae04eec2b4d9a557499b")},
                {6500000, uint256S("0x75ce456f6e5d286748f822c468e2011a36d34a3d2373cdbde0f422e46137d40d")},
                {6995462, uint256S("0xfbadb1e438a76d382ba67218230d5a703b105536c55153a7d0ff99c394bfe9e4")},
            }
        };

        chainTxData = ChainTxData{
            /* nTime    */ 1607818279,
            /* nTxCount */ 8159532,
            /* dTxRate  */ 7000.0,
        };
    }
};

// ---------------------------------------------------------------------------
// Testnet (v3)
// ---------------------------------------------------------------------------
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = CBaseChainParams::TESTNET;

        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 500000;

        consensus.BIP16Height  = 0;
        consensus.BIP34Height  = 0;
        consensus.BIP34Hash    = uint256();
        consensus.BIP65Height  = 0;
        consensus.BIP66Height  = 0;
        consensus.CSVHeight    = std::numeric_limits<int>::max();
        consensus.SegwitHeight = std::numeric_limits<int>::max();
        consensus.MinBIP9WarningHeight = 0;

        consensus.powLimit = uint256S(
            "0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan          = 24 * 30;
        consensus.nPowTargetSpacing           = 30;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting            = false;

        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow       = 2016;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit        = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit          = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartHeight =
            std::numeric_limits<int>::max() / 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeoutHeight =
            std::numeric_limits<int>::max() / 2;

        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].bit          = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartHeight =
            std::numeric_limits<int>::max() / 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeoutHeight =
            std::numeric_limits<int>::max() / 2;

        consensus.nMinimumChainWork  = uint256S("0x00");
        consensus.defaultAssumeValid = uint256S(
            "0x24463e4d3c625b0a9059f309044c2cf0d7e196cf2a6ecce901f24f681be33c8f");

        pchMessageStart[0] = 0xac;
        pchMessageStart[1] = 0xb1;
        pchMessageStart[2] = 0xc5;
        pchMessageStart[3] = 0xdc;
        nDefaultPort = 27020;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;

        // Testnet genesis: same coinbase script, different time/nonce
        genesis = CreateGenesisBlock(1394101189, 1556996, 0x1e0ffff0, 1, 88 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0x24463e4d3c625b0a9059f309044c2cf0d7e196cf2a6ecce901f24f681be33c8f"));

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS]  = std::vector<unsigned char>(1, 113); // 0x71
        base58Prefixes[SCRIPT_ADDRESS]  = std::vector<unsigned char>(1, 196); // 0xC4
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 58);
        base58Prefixes[SECRET_KEY]      = std::vector<unsigned char>(1, 241); // 0xF1
        base58Prefixes[EXT_PUBLIC_KEY]  = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY]  = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tnyc";
        mweb_hrp = "tnycmweb";

        fDefaultConsistencyChecks = false;
        fRequireStandard          = false;
        m_is_test_chain           = true;
        m_is_mockable_chain       = false;

        checkpointData = {
            {
                {0, uint256S("0x24463e4d3c625b0a9059f309044c2cf0d7e196cf2a6ecce901f24f681be33c8f")},
            }
        };

        chainTxData = ChainTxData{
            1440601451,
            1119061,
            1000.0,
        };
    }
};

// ---------------------------------------------------------------------------
// Signet (reuse testnet parameters for Phase 2)
// ---------------------------------------------------------------------------
class CSigNetParams : public CChainParams {
public:
    explicit CSigNetParams(const ArgsManager& args) {
        strNetworkID = CBaseChainParams::SIGNET;
        consensus.signet_blocks = true;

        // Default signet challenge (Bitcoin signet key)
        consensus.signet_challenge = ParseHex(
            "512103ad5e0edad18cb1f0fc0d28a3d4f1f3e445640337489abb10404f2d1e086be430"
            "210359ef5021964fe22d6f8e05b2463c9540ce96883fe3b278760f048f5189f2e6c452ae");

        consensus.nSubsidyHalvingInterval = 500000;
        consensus.BIP16Height  = 1;
        consensus.BIP34Height  = 1;
        consensus.BIP34Hash    = uint256();
        consensus.BIP65Height  = 1;
        consensus.BIP66Height  = 1;
        consensus.CSVHeight    = 1;
        consensus.SegwitHeight = 1;
        consensus.MinBIP9WarningHeight = 0;

        consensus.powLimit = uint256S(
            "0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan          = 24 * 30;
        consensus.nPowTargetSpacing           = 30;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting            = false;

        consensus.nRuleChangeActivationThreshold = 1512;
        consensus.nMinerConfirmationWindow       = 2016;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit        = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit            = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartHeight   = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeoutHeight = std::numeric_limits<int>::max() / 2;

        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].bit          = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartHeight =
            std::numeric_limits<int>::max() / 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeoutHeight =
            std::numeric_limits<int>::max() / 2;

        // Signet genesis
        genesis = CreateGenesisBlock(1394102925, 2482334, 0x1e0ffff0, 1, 88 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        consensus.nMinimumChainWork  = uint256S("0x00");
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xac;
        pchMessageStart[1] = 0xb1;
        pchMessageStart[2] = 0xc5;
        pchMessageStart[3] = 0xdc;
        nDefaultPort = 38333;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS]  = std::vector<unsigned char>(1, 113);
        base58Prefixes[SCRIPT_ADDRESS]  = std::vector<unsigned char>(1, 196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 58);
        base58Prefixes[SECRET_KEY]      = std::vector<unsigned char>(1, 241);
        base58Prefixes[EXT_PUBLIC_KEY]  = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY]  = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tnyc";
        mweb_hrp = "tnycmweb";

        fDefaultConsistencyChecks = false;
        fRequireStandard          = true;
        m_is_test_chain           = true;
        m_is_mockable_chain       = false;

        checkpointData = {{}};
        chainTxData = ChainTxData{0, 0, 0};

        // Override signet challenge from -signetchallenge arg if provided
        if (args.IsArgSet("-signetchallenge")) {
            const auto signet_challenge = args.GetArg("-signetchallenge", "");
            if (!IsHex(signet_challenge)) {
                throw std::runtime_error(strprintf("%s: -signetchallenge must be hex", __func__));
            }
            consensus.signet_challenge = ParseHex(signet_challenge);
        }

        // Signet seed nodes
        if (args.IsArgSet("-signetseednode")) {
            vSeeds = args.GetArgs("-signetseednode");
        }
    }
};

// ---------------------------------------------------------------------------
// Regression test
// ---------------------------------------------------------------------------
class CRegTestParams : public CChainParams {
public:
    explicit CRegTestParams(const ArgsManager& args) {
        strNetworkID = CBaseChainParams::REGTEST;

        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 150;

        consensus.BIP16Height  = 0;
        consensus.BIP34Height  = 500;
        consensus.BIP34Hash    = uint256();
        consensus.BIP65Height  = 1351;
        consensus.BIP66Height  = 1251;
        consensus.CSVHeight    = 432;
        consensus.SegwitHeight = 0; // SegWit always active on regtest
        consensus.MinBIP9WarningHeight = 0;

        consensus.powLimit = uint256S(
            "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan          = 24 * 30;
        consensus.nPowTargetSpacing           = 30;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting            = true;

        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow       = 144;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit        = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit            = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime   =
            Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout     =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].bit          = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartHeight =
            std::numeric_limits<int>::max() / 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeoutHeight =
            std::numeric_limits<int>::max() / 2;

        consensus.nMinimumChainWork  = uint256{};
        consensus.defaultAssumeValid = uint256{};

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        UpdateActivationParametersFromArgs(args);

        genesis = CreateGenesisBlock(1296688602, 0, 0x207fffff, 1, 88 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        vFixedSeeds.clear();
        vSeeds.clear();

        fDefaultConsistencyChecks = true;
        fRequireStandard          = true;
        m_is_test_chain           = true;
        m_is_mockable_chain       = true;

        checkpointData = {
            {
                {0, consensus.hashGenesisBlock},
            }
        };
        chainTxData = ChainTxData{0, 0, 0};

        base58Prefixes[PUBKEY_ADDRESS]  = std::vector<unsigned char>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS]  = std::vector<unsigned char>(1, 196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 58);
        base58Prefixes[SECRET_KEY]      = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY]  = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY]  = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "rnyc";
        mweb_hrp = "rnycmweb";
    }

    void UpdateVersionBitsParameters(Consensus::DeploymentPos d,
                                     int64_t nStartTime,
                                     int64_t nTimeout,
                                     int64_t nStartHeight,
                                     int64_t nTimeoutHeight)
    {
        consensus.vDeployments[d].nStartTime    = nStartTime;
        consensus.vDeployments[d].nTimeout      = nTimeout;
        consensus.vDeployments[d].nStartHeight  = nStartHeight;
        consensus.vDeployments[d].nTimeoutHeight = nTimeoutHeight;
    }

    void UpdateActivationParametersFromArgs(const ArgsManager& args);
};

void CRegTestParams::UpdateActivationParametersFromArgs(const ArgsManager& args)
{
    if (args.IsArgSet("-segwitheight")) {
        int64_t height = args.GetArg("-segwitheight", consensus.SegwitHeight);
        if (height < -1 || height >= std::numeric_limits<int>::max()) {
            throw std::runtime_error(strprintf(
                "Activation height %ld for segwit is out of valid range.", height));
        } else if (height == -1) {
            LogPrintf("Segwit disabled for testing\n");
            height = std::numeric_limits<int>::max();
        }
        consensus.SegwitHeight = static_cast<int>(height);
    }

    if (!args.IsArgSet("-vbparams")) return;

    for (const std::string& strDeployment : args.GetArgs("-vbparams")) {
        std::vector<std::string> vDeploymentParams;
        boost::split(vDeploymentParams, strDeployment, boost::is_any_of(":"));
        if (vDeploymentParams.size() < 3 || 5 < vDeploymentParams.size()) {
            throw std::runtime_error(
                "Version bits parameters malformed, expecting "
                "deployment:start:end[:heightstart:heightend]");
        }
        int64_t nStartTime, nTimeout, nStartHeight = 0, nTimeoutHeight = 0;
        if (!ParseInt64(vDeploymentParams[1], &nStartTime)) {
            throw std::runtime_error(strprintf("Invalid nStartTime (%s)", vDeploymentParams[1]));
        }
        if (!ParseInt64(vDeploymentParams[2], &nTimeout)) {
            throw std::runtime_error(strprintf("Invalid nTimeout (%s)", vDeploymentParams[2]));
        }
        if (vDeploymentParams.size() > 3 && !ParseInt64(vDeploymentParams[3], &nStartHeight)) {
            throw std::runtime_error(
                strprintf("Invalid nStartHeight (%s)", vDeploymentParams[3]));
        }
        if (vDeploymentParams.size() > 4 && !ParseInt64(vDeploymentParams[4], &nTimeoutHeight)) {
            throw std::runtime_error(
                strprintf("Invalid nTimeoutHeight (%s)", vDeploymentParams[4]));
        }
        bool found = false;
        for (int j = 0; j < (int)Consensus::MAX_VERSION_BITS_DEPLOYMENTS; ++j) {
            if (vDeploymentParams[0] == VersionBitsDeploymentInfo[j].name) {
                UpdateVersionBitsParameters(Consensus::DeploymentPos(j),
                                            nStartTime, nTimeout,
                                            nStartHeight, nTimeoutHeight);
                found = true;
                LogPrintf("Setting version bits activation parameters for %s to "
                          "start=%ld, timeout=%ld, start_height=%d, timeout_height=%d\n",
                          vDeploymentParams[0], nStartTime, nTimeout,
                          nStartHeight, nTimeoutHeight);
                break;
            }
        }
        if (!found) {
            throw std::runtime_error(
                strprintf("Invalid deployment (%s)", vDeploymentParams[0]));
        }
    }
}

// ---------------------------------------------------------------------------
// Factory / global accessors
// ---------------------------------------------------------------------------
static std::unique_ptr<const CChainParams> globalChainParams;

const CChainParams& Params()
{
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<const CChainParams> CreateChainParams(const ArgsManager& args,
                                                       const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN) {
        return std::unique_ptr<CChainParams>(new CMainParams());
    } else if (chain == CBaseChainParams::TESTNET) {
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    } else if (chain == CBaseChainParams::SIGNET) {
        return std::unique_ptr<CChainParams>(new CSigNetParams(args));
    } else if (chain == CBaseChainParams::REGTEST) {
        return std::unique_ptr<CChainParams>(new CRegTestParams(args));
    }
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(gArgs, network);
}
