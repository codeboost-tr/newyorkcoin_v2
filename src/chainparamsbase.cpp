// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Copyright (c) 2011-2021 The Litecoin Core developers
// Copyright (c) 2013-2026 The NewYorkCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparamsbase.h>

#include <tinyformat.h>
#include <util/system.h>

#include <assert.h>

// ---------------------------------------------------------------------------
// Default data directory suffixes per chain
// ---------------------------------------------------------------------------
const std::string CBaseChainParams::MAIN    = "main";
const std::string CBaseChainParams::TESTNET = "test";
const std::string CBaseChainParams::SIGNET  = "signet";
const std::string CBaseChainParams::REGTEST = "regtest";

void SetupChainParamsBaseOptions(ArgsManager& argsman)
{
    argsman.AddArg("-chain=<chain>",
                   "Use the chain <chain> (default: main). Allowed values: main, test, signet, regtest",
                   ArgsManager::ALLOW_ANY, OptionsCategory::CHAINPARAMS);
    argsman.AddArg("-regtest",
                   "Enter regression test mode, which uses a special chain in which blocks can be "
                   "solved instantly. This is intended for regression testing tools and app development. "
                   "Equivalent to -chain=regtest.",
                   ArgsManager::ALLOW_ANY | ArgsManager::DEBUG_ONLY, OptionsCategory::CHAINPARAMS);
    argsman.AddArg("-signet",
                   "Use the signet chain. Equivalent to -chain=signet. Note that the network is "
                   "defined by the -signetchallenge parameter.",
                   ArgsManager::ALLOW_ANY, OptionsCategory::CHAINPARAMS);
    argsman.AddArg("-signetchallenge",
                   "Blocks must satisfy the given script to be considered valid (only for signet "
                   "networks; defaults to the global default signet test network challenge). "
                   "To use a custom signet, pass -signetchallenge=<hex script>.",
                   ArgsManager::ALLOW_ANY, OptionsCategory::CHAINPARAMS);
    argsman.AddArg("-signetseednode",
                   "Specify a seed node for the signet network, in the hostname[:port] format, "
                   "e.g. sig.example.com or 2001:db8:85a3::8a2e:370:7334:38333. Repeat option for "
                   "multiple values. Multiple values result in outgoing connections from signet to "
                   "those nodes only.",
                   ArgsManager::ALLOW_ANY, OptionsCategory::CHAINPARAMS);
    argsman.AddArg("-testnet",
                   "Use the test chain. Equivalent to -chain=test.",
                   ArgsManager::ALLOW_ANY, OptionsCategory::CHAINPARAMS);
}

static std::unique_ptr<CBaseChainParams> globalChainBaseParams;

const CBaseChainParams& BaseParams()
{
    assert(globalChainBaseParams);
    return *globalChainBaseParams;
}

/**
 * Port assignments for NewYorkCoin Core v2.0
 *
 * Chain     | RPC port | P2P port
 * ----------+----------+---------
 * mainnet   |  17021   |  17020
 * testnet   |  27021   |  27020
 * signet    |  39332   |  38333   (reuse litecoin signet defaults)
 * regtest   |  18443   |  18444
 */
std::unique_ptr<CBaseChainParams> CreateBaseChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN) {
        // RPC 17021, onion/P2P 17020
        return MakeUnique<CBaseChainParams>("", 17021, 17020);
    } else if (chain == CBaseChainParams::TESTNET) {
        return MakeUnique<CBaseChainParams>("testnet4", 27021, 27020);
    } else if (chain == CBaseChainParams::SIGNET) {
        return MakeUnique<CBaseChainParams>("signet", 39332, 38333);
    } else if (chain == CBaseChainParams::REGTEST) {
        return MakeUnique<CBaseChainParams>("regtest", 18443, 18444);
    }
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectBaseParams(const std::string& chain)
{
    globalChainBaseParams = CreateBaseChainParams(chain);
    gArgs.SelectConfigNetwork(chain);
}
