#pragma once

#include <string>

namespace fmm::domain {

struct PackageMetadata {
  std::string name;
  std::string version;
  std::string source;

  auto operator==(const PackageMetadata&) const -> bool = default;
};

} // namespace fmm::domain
