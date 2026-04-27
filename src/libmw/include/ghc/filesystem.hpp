#pragma once
// Compatibility shim: map ghc::filesystem to std::filesystem (C++17)
// Used in place of https://github.com/gulrak/filesystem when building on MSVC.

#include <filesystem>

namespace ghc {
    namespace filesystem = std::filesystem;
}
