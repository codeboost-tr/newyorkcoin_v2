#pragma once
// Compatibility shim: provides mio::mmap_source using Windows memory-mapped file API.
// Only the subset used by mw/file/MemMap.h is implemented.
#include <cstddef>
#include <cstdint>
#include <string>
#include <system_error>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace mio {

class mmap_source {
public:
    mmap_source() = default;

    ~mmap_source() { unmap(); }

    // Non-copyable
    mmap_source(const mmap_source&) = delete;
    mmap_source& operator=(const mmap_source&) = delete;

    // Movable
    mmap_source(mmap_source&& o) noexcept
        : m_data(o.m_data), m_size(o.m_size),
          m_file(o.m_file), m_mapping(o.m_mapping)
    {
        o.m_data = nullptr;
        o.m_size = 0;
        o.m_file = INVALID_HANDLE_VALUE;
        o.m_mapping = nullptr;
    }

    mmap_source& operator=(mmap_source&& o) noexcept
    {
        if (this != &o) {
            unmap();
            m_data    = o.m_data;    o.m_data    = nullptr;
            m_size    = o.m_size;    o.m_size    = 0;
            m_file    = o.m_file;    o.m_file    = INVALID_HANDLE_VALUE;
            m_mapping = o.m_mapping; o.m_mapping = nullptr;
        }
        return *this;
    }

    void unmap() noexcept
    {
        if (m_data)    { UnmapViewOfFile(m_data);   m_data    = nullptr; }
        if (m_mapping) { CloseHandle(m_mapping);    m_mapping = nullptr; }
        if (m_file != INVALID_HANDLE_VALUE) { CloseHandle(m_file); m_file = INVALID_HANDLE_VALUE; }
        m_size = 0;
    }

    bool        empty()   const noexcept { return m_size == 0; }
    std::size_t size()    const noexcept { return m_size; }
    const char* cbegin()  const noexcept { return static_cast<const char*>(m_data); }
    const char* cend()    const noexcept { return static_cast<const char*>(m_data) + m_size; }

    // Internal state — set by make_mmap_source
    HANDLE      m_file    {INVALID_HANDLE_VALUE};
    HANDLE      m_mapping {nullptr};
    void*       m_data    {nullptr};
    std::size_t m_size    {0};
};

inline mmap_source make_mmap_source(const std::string& path, std::error_code& ec) noexcept
{
    mmap_source result;
    ec.clear();

    HANDLE hFile = CreateFileA(
        path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        ec = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        return result;
    }

    LARGE_INTEGER fileSize{};
    if (!GetFileSizeEx(hFile, &fileSize)) {
        ec = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        CloseHandle(hFile);
        return result;
    }

    if (fileSize.QuadPart == 0) {
        // Zero-length file: leave data/mapping null, keep file closed
        CloseHandle(hFile);
        return result;
    }

    HANDLE hMapping = CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMapping) {
        ec = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        CloseHandle(hFile);
        return result;
    }

    void* data = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!data) {
        ec = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return result;
    }

    result.m_file    = hFile;
    result.m_mapping = hMapping;
    result.m_data    = data;
    result.m_size    = static_cast<std::size_t>(fileSize.QuadPart);
    return result;
}

} // namespace mio
