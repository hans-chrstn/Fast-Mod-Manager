#pragma once

#include "domain/PackageDependencyGraph.hpp"
#include "domain/PackagePlan.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace fmm::test::namespace_fixtures {

struct PlanningFixture final {
  std::string profile_name;
  std::vector<domain::PackagePlan> package_plans;
  std::vector<domain::PackageId> enabled_packages;
  std::vector<domain::PackageDependency> dependencies;
  std::vector<domain::PackageOrderConstraint> ordering_constraints;
  std::vector<domain::PackageId> priority_order;
};

struct LargeConflictFixtureSize final {
  std::size_t package_count;
  std::size_t path_count;
};

inline auto mappings(std::vector<std::pair<std::string, std::string>> roots)
    -> std::vector<domain::PackageRootMapping> {
  std::vector<domain::PackageRootMapping> result;
  result.reserve(roots.size());
  for (auto& [package_root, game_root] : roots) {
    result.emplace_back(domain::PackageRelativePath{std::move(package_root)},
                        domain::GameRelativePath{std::move(game_root)});
  }
  return result;
}

inline auto entry(std::string path,
                  domain::PackageEntryKind kind = domain::PackageEntryKind::RegularFile)
    -> domain::PackageLayoutEntry {
  return domain::PackageLayoutEntry{domain::PackageRelativePath{std::move(path)}, kind};
}

inline auto ordinaryFixture() -> PlanningFixture {
  std::string arbitrary_path{"Data/bytes/"};
  arbitrary_path.push_back(static_cast<char>(0xFE));

  std::vector<domain::PackagePlan> plans;
  plans.emplace_back(
      domain::PackageId{"low"},
      domain::PackageLocation{.staging_path = std::filesystem::path{"/fixtures/staging/low"}},
      std::vector<domain::PackageLayoutEntry>{
          entry("Root", domain::PackageEntryKind::Directory), entry("Root/loader.so"),
          entry("Data", domain::PackageEntryKind::Directory), entry("Data/textures/shared.dds"),
          entry("Data/meshes/low.nif"),
          entry("Data/links/current", domain::PackageEntryKind::SymbolicLink)},
      mappings({{"Root", ""}, {"Data", "Data"}}));
  plans.emplace_back(
      domain::PackageId{"high"}, domain::PackageLocation{.object_store_ref = "object:high"},
      std::vector<domain::PackageLayoutEntry>{
          entry("Root", domain::PackageEntryKind::Directory), entry("Root/loader.so"),
          entry("Data", domain::PackageEntryKind::Directory), entry("Data/textures/shared.dds"),
          entry("Data/meshes/high.nif"), entry(std::move(arbitrary_path))},
      mappings({{"Root", ""}, {"Data", "Data"}}));

  return PlanningFixture{
      .profile_name = "Synthetic Profile",
      .package_plans = std::move(plans),
      .enabled_packages = {domain::PackageId{"high"}, domain::PackageId{"low"}},
      .dependencies = {domain::PackageDependency{domain::PackageId{"high"},
                                                 domain::PackageId{"low"}}},
      .ordering_constraints = {},
      .priority_order = {domain::PackageId{"low"}, domain::PackageId{"high"}},
  };
}

inline auto emptyFixture() -> PlanningFixture {
  return PlanningFixture{.profile_name = "Empty Synthetic Profile"};
}

inline auto obstructionFixture() -> PlanningFixture {
  std::vector<domain::PackagePlan> plans;
  plans.emplace_back(
      domain::PackageId{"obstruction"},
      domain::PackageLocation{.object_store_ref = "object:obstruction"},
      std::vector<domain::PackageLayoutEntry>{entry("Data/blocker"), entry("Data/blocker/child")},
      mappings({{"Data", "Data"}}));
  return PlanningFixture{
      .profile_name = "Obstruction Fixture",
      .package_plans = std::move(plans),
      .enabled_packages = {domain::PackageId{"obstruction"}},
      .priority_order = {domain::PackageId{"obstruction"}},
  };
}

inline auto ambiguityFixture() -> PlanningFixture {
  std::vector<domain::PackagePlan> plans;
  plans.emplace_back(
      domain::PackageId{"ambiguous"},
      domain::PackageLocation{.staging_path = std::filesystem::path{"/fixtures/ambiguous"}},
      std::vector<domain::PackageLayoutEntry>{
          entry("First/shared"), entry("Second/shared", domain::PackageEntryKind::SymbolicLink)},
      mappings({{"First", "Data"}, {"Second", "Data"}}));
  return PlanningFixture{
      .profile_name = "Ambiguity Fixture",
      .package_plans = std::move(plans),
      .enabled_packages = {domain::PackageId{"ambiguous"}},
      .priority_order = {domain::PackageId{"ambiguous"}},
  };
}

inline auto largeConflictFixture(LargeConflictFixtureSize size) -> PlanningFixture {
  PlanningFixture fixture{.profile_name = "Large Conflict Fixture"};
  fixture.package_plans.reserve(size.package_count);
  fixture.enabled_packages.reserve(size.package_count);
  fixture.priority_order.reserve(size.package_count);

  for (std::size_t package_index = 0; package_index < size.package_count; ++package_index) {
    auto package_name = std::string{"package-"} + std::to_string(package_index);
    std::vector<domain::PackageLayoutEntry> entries;
    entries.reserve(size.path_count + 1);
    entries.push_back(entry("Data", domain::PackageEntryKind::Directory));
    for (std::size_t path_index = 0; path_index < size.path_count; ++path_index) {
      entries.push_back(entry(std::string{"Data/shared/file-"} + std::to_string(path_index)));
    }

    fixture.package_plans.emplace_back(
        domain::PackageId{package_name},
        domain::PackageLocation{.object_store_ref = std::string{"object:"} + package_name},
        std::move(entries), mappings({{"Data", "Data"}}));
    fixture.enabled_packages.emplace_back(package_name);
    fixture.priority_order.emplace_back(std::move(package_name));
  }

  return fixture;
}

} // namespace fmm::test::namespace_fixtures
