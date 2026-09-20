#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace fmm::domain {

struct PackageLocation {
  std::optional<std::filesystem::path> staging_path;
  std::optional<std::string> object_store_ref;

  auto operator==(const PackageLocation&) const -> bool = default;
};

} // namespace fmm::domain
