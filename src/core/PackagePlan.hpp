#pragma once

#include <string>
#include <vector>

namespace fmm::core {

struct PackagePlan {
  std::string package_id;
  std::string source_path;
  std::string destination_staging_path;
};

} // namespace fmm::core
