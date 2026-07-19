NewYorkCoin Core
================

Setup
---------------------
NewYorkCoin Core is the reference NewYorkCoin client and it builds the backbone of the network. It downloads and, by default, stores the entire history of NewYorkCoin transactions, which requires several gigabytes of disk space. Depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more.

To download NewYorkCoin Core, visit the [GitHub Releases page](https://github.com/jamesburrell2/newyorkcoin_v2/releases). For more information see [paywith.nyc](https://paywith.nyc/).

Running
---------------------
The following are some helpful notes on how to run NewYorkCoin Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/nyc-qt` (GUI) or
- `bin/nycd` (headless)

### Windows

Unpack the files into a directory, and then run nyc-qt.exe.

### macOS

Drag NewYorkCoin Core to your applications folder, and then run NewYorkCoin Core.

### Need Help?

* See the documentation in this repository and the [project README](/README.md) for help and more information.
* Report bugs or ask questions in the [GitHub issue tracker](https://github.com/jamesburrell2/newyorkcoin_v2/issues).

Building
---------------------
The following are developer notes on how to build NewYorkCoin Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [FreeBSD Build Notes](build-freebsd.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [NetBSD Build Notes](build-netbsd.md)

Development
---------------------
The NewYorkCoin repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Productivity Notes](productivity.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://doxygen.bitcoincore.org/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [JSON-RPC Interface](JSON-RPC-interface.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)

### Resources
* Report bugs and discuss development in the [GitHub issue tracker](https://github.com/jamesburrell2/newyorkcoin_v2/issues).

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [newyorkcoin.conf Configuration File](newyorkcoin-conf.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [Reduce Memory](reduce-memory.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [ZMQ](zmq.md)
- [PSBT support](psbt.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
