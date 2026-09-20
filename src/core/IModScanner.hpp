#pragma once

#include "domain/InstalledPackage.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <stop_token>
#include <string>
#include <vector>

namespace fmm::core {

enum class ScanError : std::uint8_t { DirectoryNotFound, PermissionDenied, Cancelled, Unknown };

class IModScanner {
public:
  virtual ~IModScanner() = default;

  [[nodiscard]] virtual auto
  scanDirectory(const std::filesystem::path& staging_dir, const std::stop_token& stoken = {},
                const std::function<void(int, const std::string&)>& progress_callback = {}) const
      -> std::expected<std::vector<domain::InstalledPackage>, ScanError> = 0;
};

} // namespace fmm::core
