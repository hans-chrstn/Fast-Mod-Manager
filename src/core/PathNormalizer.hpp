#pragma once

#include <cstdint>
#include <expected>
#include <string>

namespace fmm::core {

enum class PathError : std::uint8_t { AbsoluteNotAllowed, TraversalNotAllowed, InvalidCharacters };

class PathNormalizer {
public:
  [[nodiscard]] static auto normalizeArchiveEntry(const std::string& raw_path)
      -> std::expected<std::string, PathError>;
};

} // namespace fmm::core
