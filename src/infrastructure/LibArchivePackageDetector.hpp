#pragma once

#include "core/IPackageDetector.hpp"

namespace fmm::infrastructure {

class LibArchivePackageDetector : public core::IPackageDetector {
public:
  [[nodiscard]] auto detectFormat(const std::filesystem::path& file_path,
                                  std::error_code& error_code) const
      -> domain::PackageIdentity override;
};

} // namespace fmm::infrastructure
