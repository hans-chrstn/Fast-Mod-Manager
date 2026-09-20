#pragma once

#include "application/ports/IStagingRepository.hpp"

#include <filesystem>

namespace fmm::infrastructure {

class StagingRepositoryImpl : public application::ports::IStagingRepository {
public:
  explicit StagingRepositoryImpl(std::filesystem::path staging_root);
  ~StagingRepositoryImpl() override = default;

  [[nodiscard]] auto stagePackage(const std::filesystem::path& source_path,
                                  const domain::StagingPolicy& policy,
                                  const ProgressCallback& progress = nullptr)
      -> std::expected<domain::InstalledPackage, domain::ImportError> override;

private:
  std::filesystem::path m_staging_root;
};

} // namespace fmm::infrastructure
