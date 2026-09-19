#include "infrastructure/DependencyValidatorImpl.hpp"

#include <archive.h>

namespace fmm::infrastructure {

auto DependencyValidatorImpl::getMissingDependencies() const -> std::vector<core::DependencyIssue> {
  std::vector<core::DependencyIssue> issues;

  constexpr int min_libarchive_version = 3000000;
  if (archive_version_number() < min_libarchive_version) {
    issues.push_back({"libarchive", "Outdated libarchive version detected.",
                      "Please upgrade libarchive to version 3.0 or higher."});
  }

  return issues;
}

} // namespace fmm::infrastructure
