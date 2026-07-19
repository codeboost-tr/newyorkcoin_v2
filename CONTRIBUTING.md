Contributing to NewYorkCoin Core
================================

The NewYorkCoin Core project operates an open contributor model where anyone is
welcome to contribute towards development in the form of peer review, testing
and patches. This document explains the practical process and guidelines for
contributing.

NewYorkCoin Core is a fork of [Litecoin Core](https://github.com/litecoin-project/litecoin),
which is itself a fork of [Bitcoin Core](https://github.com/bitcoin/bitcoin).
Much of the codebase, tooling and coding conventions come from upstream, so
upstream documentation and developer notes remain a useful reference. This
guide describes how *this* repository is run.

Repository Layout
-----------------

- Repository: <https://github.com/jamesburrell2/newyorkcoin_v2>
- Main development branch: **`nyc-core-v2.0`** (this is the default branch and
  the target for all pull requests). There is no separate GUI repository — GUI
  (`src/qt`) changes go to this repository like any other change.

Getting Started
---------------

New contributors are very welcome and needed.

Reviewing and testing is highly valued and the most effective way you can
contribute as a new contributor. It also teaches you much more about the code
and process than opening pull requests. Please refer to the
[peer review](#peer-review) section below.

Before you start contributing, familiarize yourself with the NewYorkCoin Core
build system and tests. See `doc/build-*.md` for building, and
`test/functional/README.md` and `src/test/README.md` for running the unit,
functional, and fuzz tests.

If you are looking for somewhere to start, check the
[open issues](https://github.com/jamesburrell2/newyorkcoin_v2/issues). You do
not need to request permission to start working on an issue, but you are
encouraged to leave a comment if you are planning to work on it so others know
it is being addressed.

Communication Channels
----------------------

Discussion about codebase improvements happens in GitHub
[issues](https://github.com/jamesburrell2/newyorkcoin_v2/issues) and
[pull requests](https://github.com/jamesburrell2/newyorkcoin_v2/pulls).

Complicated or controversial consensus or P2P protocol changes should be raised
and discussed in a GitHub issue **before** working on a patch set, so that the
approach can be agreed on early.

Security-sensitive reports must **not** be filed as public issues — follow
[SECURITY.md](SECURITY.md) instead.

Contributor Workflow
--------------------

The codebase is maintained using the "contributor workflow" where everyone
without exception contributes patch proposals using "pull requests" (PRs). This
facilitates social contribution, easy testing and peer review.

To contribute a patch, the workflow is as follows:

  1. Fork the repository ([only for the first time](https://help.github.com/en/articles/fork-a-repo))
  2. Create a topic branch off `nyc-core-v2.0`
  3. Commit patches
  4. Push changes to your fork
  5. Open a pull request against `nyc-core-v2.0`

The project coding conventions in the [developer notes](doc/developer-notes.md)
must be followed.

### Committing Patches

In general, [commits should be atomic](https://en.wikipedia.org/wiki/Atomic_commit#Atomic_commit_convention)
and diffs should be easy to read. For this reason, do not mix any formatting
fixes or code moves with actual code changes.

Make sure each individual commit is hygienic: that it builds successfully on its
own without warnings, errors, regressions, or test failures.

Commit messages should be verbose by default, consisting of a short subject line
(50 chars max), a blank line and detailed explanatory text as separate
paragraph(s), unless the title alone is self-explanatory (like "Corrected typo
in init.cpp"), in which case a single title line is sufficient. Commit messages
should be helpful to people reading your code in the future, so explain the
reasoning for your decisions. Further explanation
[here](https://chris.beams.io/posts/git-commit/).

If a particular commit references another issue, please add the reference. For
example: `refs #1234` or `fixes #4321`. Using the `fixes` or `closes` keywords
will cause the corresponding issue to be closed when the pull request is merged.

Commit messages should never contain any `@` mentions (usernames prefixed with
"@").

Please refer to the [Git manual](https://git-scm.com/doc) for more information
about Git.

### Creating the Pull Request

The title of the pull request should be prefixed by the component or area that
the pull request affects. Valid areas are:

  - `consensus` for changes to consensus critical code
  - `doc` for changes to the documentation
  - `qt` or `gui` for changes to nyc-qt
  - `log` for changes to log messages
  - `mining` for changes to the mining code
  - `net` or `p2p` for changes to the peer-to-peer network code
  - `refactor` for structural changes that do not change behavior
  - `rpc`, `rest` or `zmq` for changes to the RPC, REST or ZMQ APIs
  - `script` for changes to the scripts and tools
  - `test`, `qa` or `ci` for changes to the unit tests, QA tests or CI code
  - `util` or `lib` for changes to the utils or libraries
  - `wallet` for changes to the wallet code
  - `build` for changes to the GNU Autotools or reproducible builds

Examples:

    consensus: reject AuxPoW blocks with mismatched chain ID
    net: automatically create onion service, listen on Tor
    qt: add fee bump button
    log: fix typo in log message

The body of the pull request should contain sufficient description of *what* the
patch does, and even more importantly, *why*, with justification and reasoning.
You should include references to any discussions (for example, other issues).

The description for a new pull request should not contain any `@` mentions. The
PR description will be included in the commit message when the PR is merged and
any users mentioned in the description will be notified each time a fork copies
the merge. Instead, make any username mentions in a subsequent comment to the
PR.

### Work in Progress Changes and Requests for Comments

If a pull request is not to be considered for merging (yet), please prefix the
title with [WIP] or use [task lists](https://help.github.com/articles/basic-writing-and-formatting-syntax/#task-lists)
in the body of the pull request to indicate tasks are pending.

### Address Feedback

At this stage, one should expect comments and review from other contributors.
You can add more commits to your pull request by committing them locally and
pushing to your fork until you have satisfied all feedback.

Code review is a burdensome but important part of the development process, and
as such, certain types of pull requests are rejected. In general, if the
**improvements** do not warrant the **review effort** required, the PR has a
high chance of being rejected. It is up to the PR author to convince the
reviewers that the changes warrant the review effort.

### Squashing Commits

If your pull request contains fixup commits (commits that change the same line
of code repeatedly) or too fine-grained commits, you may be asked to
[squash](https://git-scm.com/docs/git-rebase#_interactive_mode) your commits
before it will be merged. The basic squashing workflow is shown below.

    git checkout your_branch_name
    git rebase -i HEAD~n
    # n is normally the number of commits in the pull request.
    # Set commits (except the one in the first line) from 'pick' to 'squash',
    # save and quit.
    # On the next screen, edit/refine commit messages. Save and quit.
    git push -f # (force push to your fork)

Please update the resulting commit message, if needed. It should read as a
coherent message. In most cases, this means not just listing the interim
commits.

Please refrain from creating several pull requests for the same change. Use the
pull request that is already open to amend changes. This preserves the
discussion and review that happened earlier.

### Rebasing Changes

When a pull request conflicts with the target branch, you may be asked to rebase
it on top of the current `nyc-core-v2.0`. The `git rebase` command will take
care of rebuilding your commits on top of the new base.

This project aims to have a clean git history, where code changes are only made
in non-merge commits. This simplifies auditability because merge commits can be
assumed to not contain arbitrary code changes.

Pull Request Philosophy
-----------------------

Patchsets should always be focused. For example, a pull request could add a
feature, fix a bug, or refactor code; but not a mixture. Please also avoid
super pull requests which attempt to do too much, are overly large, or overly
complex, as this makes review difficult.

### Features

When adding a new feature, thought must be given to the long-term technical debt
and maintenance that feature may require after inclusion. Before proposing a new
feature that will require maintenance, please consider if you are willing to
maintain it (including bug fixing).

### Refactoring

Refactoring is a necessary part of any software project's evolution. There are
three categories of refactoring: code-only moves, code style fixes, and code
refactoring. In general, refactoring pull requests should not mix these three
kinds of activities, to keep them easy to review. In all cases, refactoring PRs
must not change the behaviour of code within the pull request (bugs must be
preserved as is).

Because this is a consensus-critical cryptocurrency codebase, refactoring of
consensus-critical code (`src/validation.*`, `src/consensus/`, the AuxPoW and
proof-of-work paths) is held to a much higher review bar.

"Decision Making" Process
-------------------------

The following applies to code changes to the NewYorkCoin Core project, and is
not to be confused with NewYorkCoin network protocol consensus changes.

Whether a pull request is merged into NewYorkCoin Core rests with the project
maintainers (see [CODEOWNERS](CODEOWNERS)).

In general, all pull requests must:

  - Have a clear use case, fix a demonstrable bug, or serve the greater good of
    the project (for example refactoring for modularisation);
  - Be well peer-reviewed;
  - Have unit tests, functional tests, and fuzz tests, where appropriate;
  - Follow code style guidelines ([C++](doc/developer-notes.md),
    [functional tests](test/functional/README.md));
  - Not break the existing test suite;
  - Where bugs are fixed, where possible, there should be unit tests
    demonstrating the bug and also proving the fix, to prevent regression;
  - Change relevant comments and documentation when behaviour of code changes.

Patches that change NewYorkCoin consensus rules (block time, proof-of-work /
KGW difficulty, AuxPoW rules, address prefixes, activation heights) are
considerably more involved than normal because they affect the entire network.
Such changes must be validated on regtest and a full testnet sync before they
can be considered for merge, and must never be merged without that validation.

### Peer Review

Anyone may participate in peer review, which is expressed by comments in the
pull request. Typically reviewers will review the code for obvious errors, test
out the patch set, and opine on its technical merits.

#### Conceptual Review

A review can be a conceptual review, where the reviewer leaves a comment:
 * `Concept (N)ACK`, meaning "I do (not) agree with the general goal of this
   pull request";
 * `Approach (N)ACK`, meaning `Concept ACK`, but "I do (not) agree with the
   approach of this change".

A `NACK` needs to include a rationale for why the change is not worthwhile.
NACKs without accompanying reasoning may be disregarded.

#### Code Review

After conceptual agreement on the change, code review can be provided. A review
begins with `ACK BRANCH_COMMIT`, where `BRANCH_COMMIT` is the top of the PR
branch, followed by a description of how the reviewer did the review:

  - "I have tested the code", involving change-specific manual testing in
    addition to running the unit, functional, or fuzz tests, and if it is not
    obvious how the manual testing was done, it should be described;
  - "I have not tested the code, but I have reviewed it and it looks OK, I agree
    it can be merged";
  - A "nit" refers to a trivial, often non-blocking issue.

Where a patch set affects consensus-critical code, the bar will be much higher
in terms of discussion and peer review requirements, keeping in mind that
mistakes could be very costly to the wider community.

Backporting
-----------

Security and bug fixes may be backported from upstream Bitcoin Core / Litecoin
Core. When backporting an upstream change, include the following metadata in the
commit body so the origin is traceable:

```
Github-Pull: <upstream repo>#<PR number>
Rebased-From: <commit hash of the original commit>
```

Release Policy
--------------

The release process is documented in
[doc/release-process.md](doc/release-process.md). The project maintainer acts as
the release manager for each NewYorkCoin Core release.

Copyright
---------

By contributing to this repository, you agree to license your work under the
MIT license unless specified otherwise in `contrib/debian/copyright` or at the
top of the file itself. Any work contributed where you are not the original
author must contain its license header with the original author(s) and source.
