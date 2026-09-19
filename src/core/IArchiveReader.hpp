#pragma once

#include "domain/VirtualTree.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <system_error>

namespace fmm::core {

enum class ArchiveError : std::uint8_t { IoError, InvalidFormat, CorruptHeader, UnsupportedFilter };

class IArchiveReader {
public:
  virtual ~IArchiveReader() = default;

  [[nodiscard]] virtual auto readTree(const std::filesystem::path& archive_path) const
      -> std::expected<domain::VirtualTree, ArchiveError> = 0;
};

} // namespace fmm::core
