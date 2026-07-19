# Security Policy

## Supported Versions

Security fixes are provided only for the most recent minor release series of
NewYorkCoin Core. Older series receive no security backports — please upgrade.

| Version   | Supported          |
|-----------|--------------------|
| 2.1.x     | :white_check_mark: |
| 2.0.x     | :x:                |
| < 2.0     | :x:                |

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub issues,
pull requests, or discussions.**

Report privately through either of these channels:

1. **GitHub Security Advisories (preferred).** Open a private report at
   [github.com/jamesburrell2/newyorkcoin_v2/security/advisories/new](https://github.com/jamesburrell2/newyorkcoin_v2/security/advisories/new).
   This keeps the report private, lets us collaborate on a fix, and coordinates
   disclosure through GitHub.
2. **Email.** Send details to **security@paywith.nyc**. Use this address for
   security reports only — it is not a support channel.

Please include as much of the following as you can:

- the type of issue (e.g. consensus split, remote crash, memory-safety bug,
  wallet key exposure, RPC authentication bypass);
- the component and source files involved (for example `validation.cpp`, the
  AuxPoW verification path, or the Qt wallet);
- the configuration required to reproduce (network: mainnet/testnet/regtest,
  build flags, OS);
- step-by-step reproduction instructions and, if possible, proof-of-concept
  code;
- the impact, including how an attacker might exploit it.

## Disclosure Process

- We aim to acknowledge a report within **72 hours**.
- We will confirm the issue, determine affected versions, and prepare a fix.
- Because NewYorkCoin shares a codebase with Bitcoin Core and Litecoin Core,
  a vulnerability here may also affect upstream or other downstream forks. We
  will coordinate disclosure responsibly and credit reporters who wish to be
  named.
- Please give us reasonable time to release a fix before any public disclosure.

## Scope

This policy covers the NewYorkCoin Core node, wallet, and Qt GUI in this
repository. Third-party services (explorers, pools, exchanges, mobile wallets)
are out of scope — contact their operators directly.
