NewYorkCoin Core v2.0
=====================

> **WARNING: EXPERIMENTAL SOFTWARE - READ BEFORE USE**
>
> This codebase is under active development and has **not** been audited for
> production use. It may contain bugs, consensus incompatibilities, or security
> vulnerabilities. **Do not use this software to store significant funds.**
> By building or running this software you accept all associated risks.

---

## What is NewYorkCoin?

NewYorkCoin (NYC) is a peer-to-peer digital currency originally launched in
March 2014. It features:

- **30-second block targets** with Kimoto Gravity Well (KGW) difficulty
  retargeting per block
- **Scrypt proof-of-work** with AuxPoW (merged mining) support — chain ID **1985** (0x07C1)
- **Legacy P2PKH addresses** starting with `R` (PUBKEY_ADDRESS = 60)
- **Ticker symbol**: NYC
- **P2P port**: 17020 (mainnet), 27020 (testnet)
- **RPC port**: 17021 (mainnet), 27021 (testnet)
- **Payment URI scheme**: `newyorkcoin:`
- **bech32 HRP**: `nyc`

For more information see [https://paywith.nyc/](https://paywith.nyc/).

---

## About NewYorkCoin Core v2.0

NewYorkCoin Core v2.0 is a **modernised full-node wallet** forked from
[Litecoin Core v0.21.4](https://github.com/litecoin-project/litecoin), which is
itself a fork of [Bitcoin Core](https://github.com/bitcoin/bitcoin).

### Goals

#### Completed

1. **Restore a maintained full-node client** — the original NYC 1.x codebase is
   based on Bitcoin Core 0.10 (2015). v2.0 brings the codebase forward to a
   modern base (0.21.x) with active upstream security backports.

2. **Full NYC network compatibility** — correct network magic
   (`0xCF 0xFE 0xC9 0xCC`), protocol version 70012, and chain parameters
   matching the live NYC mainnet (genesis block, ports, address prefixes).

3. **Clean branding** — all Litecoin/Bitcoin strings replaced with
   NewYorkCoin/NYC throughout the Qt GUI, RPC responses, signed-message magic,
   URI handling, and user-facing text.

4. **Legacy address generation** — wallet defaults to `OutputType::LEGACY` so
   receive addresses begin with `R`, matching addresses used on exchanges and
   the original 1.x wallet.

5. **Merged mining (AuxPoW)** — NYC uses chain ID **1985** (0x07C1) and follows
   the Namecoin/Dogecoin AuxPoW protocol. Pools can merge-mine NYC alongside any
   other Scrypt coin. The `getauxblock` RPC provides the work interface.

6. **Android companion wallet** — the
   [nyc-openwallet-android](https://github.com/openwalletGH/openwallet-android)
   project provides a lightweight SPV wallet that can connect to this full node.

7. **ElectrumX server** — public ElectrumX endpoint at `electrum.paywith.nyc:50002`
   (SSL) for lightweight wallet connectivity without running a full node.

#### Roadmap

8. **Rosetta API** — implement the
   [Coinbase Rosetta](https://docs.cloud.coinbase.com/rosetta/docs/welcome)
   specification to enable exchange listings and block explorer integrations.
   A Rosetta middleware service (`mesh-newyorkcoin`) is already scaffolded in
   this repository.

9. **Atomic Swaps** — cross-chain atomic swaps with Bitcoin, Litecoin, and other
   HTLC-compatible chains, enabling trustless peer-to-peer NYC exchanges without
   a centralised intermediary.

10. **NYC Ordinals** — an ordinals/inscription protocol for the NYC chain,
    analogous to Bitcoin Ordinals. NYC's 30-second blocks and low fees make it
    well-suited for high-throughput inscription use-cases.

11. **SegWit & Taproot addresses** — activate SegWit (P2WPKH/P2WSH, bech32
    `nyc1q...`) and Taproot (P2TR, bech32m `nyc1p...`) to reduce transaction fees,
    enable more complex scripts, and lay the groundwork for the Lightning Network.

12. **MimbleWimble / privacy** — integrate a MimbleWimble extension block
    (similar to Litecoin's MWEB implementation) to provide optional
    confidential transactions with hidden amounts and enhanced sender/receiver
    privacy, while remaining compatible with the existing UTXO set.

13. **Custom OP_CODE scripting** — extend NYC's script interpreter with
    domain-specific opcodes useful for DeFi-style contracts, cross-chain
    commitments, and ordinals metadata — building on the Taproot script path
    infrastructure.

14. **DNS seed infrastructure, checkpoints, and security audit** — add reliable
    DNS seeds, embed mainnet checkpoints, and commission a third-party security
    audit before recommending production use.

### Fork Lineage

```
Bitcoin Core  ->  Litecoin Core v0.21.4  ->  NewYorkCoin Core v2.0
```

Key departures from the Litecoin base:

| Parameter | Litecoin | NewYorkCoin Core v2.0 |
|-----------|----------|----------------------|
| Protocol version | 70017 | **70012** |
| Network magic | `0xFB 0xC0 0xB6 0xDB` | **`0xCF 0xFE 0xC9 0xCC`** |
| P2P port | 9333 | **17020** |
| PUBKEY_ADDRESS | 48 (`L`) | **60 (`R`)** |
| Block target | 2.5 min | **30 seconds** |
| Difficulty algo | KGW / DGW | **KGW (Kimoto Gravity Well)** |
| bech32 HRP | `ltc` | **`nyc`** |
| Merged mining | No | **AuxPoW (chain ID 1985 / 0x07C1)** |

---

## Build Instructions

See the platform-specific docs in [`doc/`](doc/):

- [Linux](doc/build-unix.md)
- [macOS](doc/build-osx.md)
- [Windows (cross-compile)](doc/build-windows.md)

Quick start on Ubuntu/Debian:

```bash
# Install dependencies
sudo apt-get install build-essential libtool autotools-dev automake pkg-config \
  bsdmainutils python3 libssl-dev libevent-dev libboost-all-dev \
  libminiupnpc-dev libzmq3-dev libqt5gui5 libqt5core5a libqt5dbus5 \
  qttools5-dev qttools5-dev-tools libdb5.3++-dev

# Configure and build
./autogen.sh
./configure --with-gui=qt5
make -j4

# Binaries produced:
#   src/nycd           (daemon)
#   src/nyc-cli        (CLI)
#   src/qt/nyc-qt      (Qt GUI wallet)
#   src/nyc-tx         (transaction utility)
#   src/nyc-wallet     (wallet utility)
```

### Connecting to the Network

Create `~/.newyorkcoin/newyorkcoin.conf` (or pass `-datadir=<path>`):

```ini
# Known active mainnet nodes (April 2026)
addnode=24.52.248.184
addnode=37.59.20.42
addnode=66.70.182.1
addnode=85.19.25.38
```

---

## Merged Mining (AuxPoW)

NewYorkCoin Core v2.0 supports **merge mining** via the AuxPoW protocol
(the same system used by Namecoin, Dogecoin, and others).

| Parameter | Value |
|-----------|-------|
| Chain ID | **1985** (0x07C1) |
| PoW algo | Scrypt (1024/1/1) |
| Block version flag | `0x100` (bit 8 of nVersion) |
| nVersion of AuxPoW block | `(1985 << 16) \| 0x100 \| base` = `0x07C10101` |

### How it works

A miner places the NYC block hash inside the coinbase scriptSig of a parent
chain block (e.g. a Litecoin or Dogecoin block) using the magic bytes
`0xfabe6d6d`.  When the parent block solves its own difficulty, the miner
submits the parent block header and coinbase merkle proof to NYC as an AuxPoW
solution.  As long as the parent block's Scrypt hash satisfies NYC's current
target, the NYC block is accepted.

### Pool integration — `getauxblock` RPC

```bash
# Create a new work item (returns block hash, target, height, etc.)
nyc-cli getauxblock

# Submit a solved AuxPoW
nyc-cli getauxblock <hash> <auxpow-hex>
```

The returned JSON contains:

| Field | Description |
|-------|-------------|
| `hash` | NYC block hash to embed in the parent coinbase |
| `chainid` | Always 1985 (0x07C1) |
| `previousblockhash` | Current chain tip |
| `coinbasevalue` | Total block reward available (satoshis) |
| `bits` | Compact difficulty target |
| `height` | Height of the next block |
| `target` | Full 256-bit PoW target (big-endian hex) |

### Coinbase commitment format

```
OP_RETURN  (or anywhere in scriptSig)
  0xfabe6d6d          -- 4-byte merged-mining magic
  <chainRoot>         -- 32 bytes: chain merkle tree root (= NYC block hash
                         when mining NYC alone, i.e. nSize=1)
  <nSize>             -- uint32 LE: 2^(chain branch height), 1 for solo NYC
  <nNonce>            -- uint32 LE: chainId % nSize, 0 for solo NYC (1985%1=0)
```

---

## Branch Structure

| Branch | Purpose |
|--------|---------|
| `nyc-core-v2.0` | Main development branch -- **this branch** |
| `master` | Upstream Litecoin v0.21.4 base (unmodified) |

---

## Current Status

| Feature | Status |
|---------|--------|
| Builds from source (Linux) | ✅ OK |
| Builds from source (Windows x64) | ✅ OK (cross-compiled via MinGW + depends/) |
| Builds from source (macOS) | ✅ OK |
| Connects to NYC mainnet peers | ✅ OK |
| Correct `R` address generation | ✅ OK |
| NYC branding throughout GUI | ✅ OK |
| KGW difficulty algorithm | ✅ OK |
| Full chain sync | ✅ Synced (height ~12.6M, April 2026) |
| AuxPoW merged mining | ✅ OK — chain ID 1985 (0x07C1), `getauxblock` RPC |
| ElectrumX public endpoint | ✅ `electrum.paywith.nyc:50002` (SSL) |
| Release binaries (Linux/Windows/macOS) | ✅ Available — see [Releases](https://github.com/jamesburrell2/newyorkcoin_v2/releases) |
| SHA256 / MD5 checksums for releases | ✅ Included in each release |
| DNS seed servers | ⚠️ Offline — use `addnode` |
| Checkpoints | 🔲 Planned |
| Rosetta API | 🔲 In progress (mesh-newyorkcoin scaffolded) |
| Atomic Swaps | 🔲 Planned |
| NYC Ordinals | 🔲 Planned |
| SegWit activation | 🔲 Planned |
| Taproot activation | 🔲 Planned (after SegWit) |
| MimbleWimble extension blocks | 🔲 Planned |
| Custom OP_CODE scripting | 🔲 Planned (post-Taproot) |
| Security audit | ❌ Not yet performed |

---

## License

NewYorkCoin Core is released under the terms of the MIT license. See
[COPYING](COPYING) for more information or see
<https://opensource.org/licenses/MIT>.

This software includes code from Bitcoin Core and Litecoin Core, both released
under the MIT license. All original copyright notices are preserved.

---

## Disclaimer

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. THE AUTHORS
AND CONTRIBUTORS ACCEPT NO LIABILITY FOR ANY LOSS OF FUNDS OR OTHER DAMAGES
ARISING FROM THE USE OF THIS SOFTWARE. THIS IS EXPERIMENTAL CODE -- USE AT
YOUR OWN RISK.
