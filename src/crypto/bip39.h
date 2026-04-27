// Copyright (c) 2024 The NewYorkCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTO_BIP39_H
#define BITCOIN_CRYPTO_BIP39_H

#include <string>
#include <vector>
#include <cstdint>

namespace bip39 {

/**
 * Generate a BIP39 mnemonic phrase.
 *
 * @param words  Number of words: 12 (128-bit entropy) or 24 (256-bit entropy).
 * @return Space-separated mnemonic string, or empty string on error.
 */
std::string GenerateMnemonic(int words = 12);

/**
 * Validate a BIP39 mnemonic (checks word list membership and checksum).
 *
 * @param mnemonic  Space-separated word list.
 * @return true if the mnemonic is valid, false otherwise.
 */
bool ValidateMnemonic(const std::string& mnemonic);

/**
 * Convert a BIP39 mnemonic to a 64-byte seed using PBKDF2-HMAC-SHA512
 * with 2048 iterations and salt = "mnemonic" + passphrase.
 *
 * This is the standard BIP39 seed derivation used by hardware wallets,
 * Electrum, and all BIP44-compliant wallets.
 *
 * @param mnemonic    Space-separated BIP39 word list (12 or 24 words).
 * @param passphrase  Optional passphrase ("extension word"). Defaults to "".
 * @return 64-byte seed vector. Returns empty vector on invalid mnemonic.
 */
std::vector<uint8_t> MnemonicToSeed(const std::string& mnemonic,
                                    const std::string& passphrase = "");

} // namespace bip39

#endif // BITCOIN_CRYPTO_BIP39_H
