#pragma once

#include "domain/PackageIdentity.hpp"

#include <filesystem>
#include <system_error>

namespace fmm::core {

class IPackageDetector {
public:
  virtual ~IPackageDetector() = default;

  [[nodiscard]] virtual auto detectFormat(const std::filesystem::path& file_path,
                                          std::error_code& error_code) const
      -> domain::PackageIdentity = 0;
};

} // namespace fmm::core
