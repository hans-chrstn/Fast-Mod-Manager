#pragma once

#include "domain/ImportError.hpp"
#include "domain/InstalledPackage.hpp"
#include "domain/StagingPolicy.hpp"

#include <expected>
#include <filesystem>
#include <functional>

namespace fmm::application::ports {

class IStagingRepository {
public:
  virtual ~IStagingRepository() = default;

  using ProgressCallback =
      std::function<void(std::uintmax_t bytes_copied, std::uintmax_t total_bytes)>;

  [[nodiscard]] virtual auto stagePackage(const std::filesystem::path& source_path,
                                          const domain::StagingPolicy& policy,
                                          const ProgressCallback& progress = nullptr)
      -> std::expected<domain::InstalledPackage, domain::ImportError> = 0;
};

} // namespace fmm::application::ports
