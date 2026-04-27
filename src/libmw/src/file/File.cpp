// Copyright (c) 2018-2021 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/file/File.h>
#include <mw/exceptions/FileException.h>

#include <fstream>
#include <ghc/filesystem.hpp>

void File::Create()
{
    if (!m_path.Exists()) {
        std::ofstream file(m_path.ToString(), std::ios::binary);
        if (!file) {
            ThrowFile_F("Failed to create file: {}", m_path);
        }
    }
}

size_t File::GetSize() const
{
    std::error_code ec;
    const auto size = ghc::filesystem::file_size(ghc::filesystem::path(m_path.ToString()), ec);
    if (ec) {
        ThrowFile_F("Error ({}) getting size of {}", ec.message(), m_path);
    }
    return static_cast<size_t>(size);
}

bool File::Exists() const
{
    return m_path.Exists();
}

std::vector<uint8_t> File::ReadBytes() const
{
    std::ifstream file(m_path.ToString(), std::ios::binary | std::ios::ate);
    if (!file) {
        ThrowFile_F("Failed to open file for reading: {}", m_path);
    }
    const auto size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> bytes(size);
    if (size > 0 && !file.read(reinterpret_cast<char*>(bytes.data()), size)) {
        ThrowFile_F("Failed to read file: {}", m_path);
    }
    return bytes;
}

std::vector<uint8_t> File::ReadBytes(const size_t startIndex, const size_t numBytes) const
{
    std::ifstream file(m_path.ToString(), std::ios::binary);
    if (!file) {
        ThrowFile_F("Failed to open file for reading: {}", m_path);
    }
    file.seekg(static_cast<std::streamoff>(startIndex), std::ios::beg);
    std::vector<uint8_t> bytes(numBytes);
    if (numBytes > 0 && !file.read(reinterpret_cast<char*>(bytes.data()), numBytes)) {
        ThrowFile_F("Failed to read {} bytes from {}", numBytes, m_path);
    }
    return bytes;
}

void File::Write(const std::vector<uint8_t>& bytes)
{
    std::ofstream file(m_path.ToString(), std::ios::binary | std::ios::trunc);
    if (!file) {
        ThrowFile_F("Failed to open file for writing: {}", m_path);
    }
    if (!bytes.empty() && !file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size())) {
        ThrowFile_F("Failed to write file: {}", m_path);
    }
}

void File::Write(const size_t startIndex, const std::vector<uint8_t>& bytes, const bool truncate)
{
    const auto flags = truncate
        ? (std::ios::binary | std::ios::in | std::ios::out)
        : (std::ios::binary | std::ios::in | std::ios::out);
    std::fstream file(m_path.ToString(), flags);
    if (!file) {
        // File may not exist yet; create it
        std::ofstream create(m_path.ToString(), std::ios::binary);
        if (!create) {
            ThrowFile_F("Failed to create file for writing: {}", m_path);
        }
        create.close();
        file.open(m_path.ToString(), flags);
        if (!file) {
            ThrowFile_F("Failed to open file for writing: {}", m_path);
        }
    }
    file.seekp(static_cast<std::streamoff>(startIndex), std::ios::beg);
    if (!bytes.empty() && !file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size())) {
        ThrowFile_F("Failed to write to file: {}", m_path);
    }
    if (truncate) {
        const auto endPos = static_cast<std::streamoff>(startIndex + bytes.size());
        file.close();
        std::error_code ec;
        ghc::filesystem::resize_file(ghc::filesystem::path(m_path.ToString()), endPos, ec);
        if (ec) {
            ThrowFile_F("Error ({}) truncating {}", ec.message(), m_path);
        }
    }
}

void File::WriteBytes(const std::unordered_map<uint64_t, uint8_t>& bytes)
{
    std::fstream file(m_path.ToString(), std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        ThrowFile_F("Failed to open file for writing bytes: {}", m_path);
    }
    for (const auto& pair : bytes) {
        file.seekp(static_cast<std::streamoff>(pair.first), std::ios::beg);
        const char byte = static_cast<char>(pair.second);
        if (!file.write(&byte, 1)) {
            ThrowFile_F("Failed to write byte at offset {} in {}", pair.first, m_path);
        }
    }
}

void File::Truncate(const uint64_t size)
{
    std::error_code ec;
    ghc::filesystem::resize_file(ghc::filesystem::path(m_path.ToString()), size, ec);
    if (ec) {
        ThrowFile_F("Error ({}) truncating {}", ec.message(), m_path);
    }
}

void File::CopyTo(const FilePath& new_path) const
{
    std::error_code ec;
    ghc::filesystem::copy_file(
        ghc::filesystem::path(m_path.ToString()),
        ghc::filesystem::path(new_path.ToString()),
        ghc::filesystem::copy_options::overwrite_existing,
        ec);
    if (ec) {
        ThrowFile_F("Error ({}) copying {} to {}", ec.message(), m_path, new_path);
    }
}
