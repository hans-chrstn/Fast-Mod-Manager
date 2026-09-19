#pragma once

#include "domain/ModIdentity.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <vector>

namespace fmm::core {

enum class ScanError : std::uint8_t { DirectoryNotFound, PermissionDenied, Unknown };

class IModScanner {
public:
  virtual ~IModScanner() = default;

  [[nodiscard]] virtual auto scanDirectory(const std::filesystem::path& staging_dir) const
      -> std::expected<std::vector<domain::ModIdentity>, ScanError> = 0;
};

} // namespace fmm::core
