# NYC Consensus Security Review — Findings Catalog

*Date: 2026-07-20 · Scope: NewYorkCoin-specific consensus surface on `nyc-core-v2.0`*

This is a **findings catalog only** — no fixes are applied here. Confirmed
fixes belong to Phase 2 (consensus test coverage), where each finding must be
reproduced on regtest and, where relevant, validated against a full testnet
sync before any change to consensus-critical code is merged.

It supersedes the two `copilot/*security-audit*` branches on the upstream repo,
which contained no committed findings (only line-ending churn); their actual
output lives only in private GitHub Copilot session logs.

Reviewed surface: `src/pow.cpp`, `src/auxpow.cpp`, `src/auxpow.h`,
`src/rpc/mining.cpp` (getauxblock/submitauxblock), and the PoW/AuxPoW call
sites in `src/validation.cpp`.

---

## #1 — HIGH — Difficulty is DigiShield-only; no algorithm for heights below 4,800,000

**Files:** `src/pow.cpp` (`GetNextWorkRequired`, `CalculateNextWorkRequired`),
`src/validation.cpp:3693` (`ContextualCheckBlockHeader`).

`GetNextWorkRequired` applies a single DigiShield retarget (2-block interval,
amplitude filter, clamp to [75 %, 150 %]) at **every** height. There is no
height gate and no alternative branch.

However `src/pow.cpp` itself documents that the live chain uses DigiShield only
*"above block 4,800,000"*, and `src/validation.cpp:1282-1288` keys the block
reward schedule to height 4,800,000 (*"Post-AuxPoW era"*). Together these
indicate v2 continues the **same chain** the v1.14.x network has run since 2014
— so it must be able to validate historical blocks below 4,800,000, which were
produced under a different retarget algorithm (the v1 code path referenced in
the `pow.cpp` header comment, i.e. KGW-era rules).

`ContextualCheckBlockHeader` recomputes and enforces `block.nBits ==
GetNextWorkRequired(...)` for **every** header (line 3693). The checkpoint block
below it only rejects forks older than the last checkpoint; it does **not**
skip the difficulty recomputation. Therefore:

**Failure scenario:** a node syncing mainnet from genesis reaches any pre-
4,800,000 block, recomputes its target with DigiShield, gets a value that does
not match the historical (non-DigiShield) `nBits`, and rejects the block with
`bad-diffbits`. The node cannot sync the historical chain, and a v2 node
disagrees with v1 nodes about the valid chain → **consensus split / migration
blocker.** This is the roadmap's #1 risk (untested consensus differences).

**To verify / resolve (Phase 2 + Phase 4):**
- Confirm the exact retarget algorithm the v1.14.x network used below height
  4,800,000 (KGW parameters, any DigiShield transition height).
- Decide and document the intended strategy, one of:
  - restore the historical algorithm behind a height gate so genesis sync
    reproduces historical `nBits`; **or**
  - launch/ship v2 from a genesis or trusted snapshot at/after 4,800,000 (a
    fresh-start decision that must be stated explicitly and reconciled with the
    "same chain since 2014" framing and v1↔v2 coexistence goal); **or**
  - introduce an explicit, reviewed checkpoint/`assumevalid` mechanism that
    bypasses historical difficulty validation — noting the current code path
    does **not** do this.
- Whichever is chosen, add a regtest/testnet regression test before touching
  `pow.cpp`.

## #2 — MEDIUM — End-of-life OpenSSL dependency pin

**File:** `depends/packages/openssl.mk` (pinned 1.0.1k, 2015, EOL);
`configure.ac` still requires libssl.

A 2015 EOL OpenSSL carries known unpatched CVEs. Even with a narrow use surface
(`src/wallet/crypter.cpp`, `src/crypto/scrypt-sse2.cpp`), shipping release
binaries linked against it is a real exposure. Tracked by roadmap Phase 3
(remove OpenSSL entirely like upstream 0.20, or move to 3.x); recorded here as a
security item so it is not lost.

## #3 — LOW — AuxPoW coinbase merkle branch length is not bounded

**File:** `src/auxpow.cpp:52` and `CAuxPow::Check` step (2) at line 57.

`Check()` caps `vChainMerkleBranch.size() > 30` to prevent resource exhaustion,
but the coinbase merkle branch `vMerkleBranch` (consumed by
`ComputeAuxMerkleRootFromBranch` at line 57) has no explicit cap. It is bounded
indirectly by transaction/deserialization limits, so this is defense-in-depth
rather than a live DoS, but the two branches should be treated consistently —
add an explicit upper bound on `vMerkleBranch.size()`.

## #4 — INFORMATIONAL — PoW / AuxPoW wiring reviewed, no issue found

The parent-PoW and scrypt wiring appears correct and is recorded as a positive
result:

- `src/validation.cpp:1197` and `:3529` call `CAuxPow::Check(block.GetHash(),
  nAuxpowChainId, params)` for merge-mined blocks.
- `:1199` / `:3535` verify the **parent** block's scrypt PoW
  (`GetParentBlockPoWHash()`) against `nBits` for AuxPoW blocks; `:1202` /
  `:3503` verify the block's own scrypt PoW for non-AuxPoW blocks.
- `CAuxPow::Check` enforces coinbase-is-first, the one-hop (parent-has-our-
  chain-id) rule, chain merkle root placement relative to the merged-mining
  header, `nSize == 2^height`, and the deterministic `GetExpectedIndex` slot —
  matching the Namecoin/Dogecoin merged-mining specification.

No fix required; listed so the reviewed-and-cleared surface is on record.

---

### Suggested handling

- **#1** is the gating item for any "safe for mainnet" claim and is Phase 2/4
  work.
- **#2** folds into Phase 3 dependency modernization.
- **#3** is a small, self-contained hardening PR that can land early (with a
  fuzz/unit test per Phase 2.5).
