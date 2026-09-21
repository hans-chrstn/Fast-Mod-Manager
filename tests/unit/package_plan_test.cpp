#include "core/IPlugin.hpp"
#include "domain/PackagePlan.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

using PackagePlanContribution = void (fmm::core::IPlugin::*)(PackagePlan&);

static_assert(std::is_enum_v<PackageEntryKind>);
static_assert(std::is_constructible_v<PackageRelativePath, std::string>);
static_assert(!std::is_convertible_v<std::string, PackageRelativePath>);
static_assert(std::is_same_v<decltype(std::declval<const PackageLayoutEntry&>().relativePath()),
                             const PackageRelativePath&>);
static_assert(
    std::is_same_v<decltype(std::declval<const PackageLayoutEntry&>().kind()), PackageEntryKind>);
static_assert(
    std::is_same_v<decltype(std::declval<const PackagePlan&>().packageId()), const PackageId&>);
static_assert(std::is_same_v<decltype(std::declval<const PackagePlan&>().packageLocation()),
                             const PackageLocation&>);
static_assert(std::is_same_v<decltype(std::declval<const PackagePlan&>().entries()),
                             const std::vector<PackageLayoutEntry>&>);
static_assert(
    std::is_same_v<decltype(&fmm::core::IPlugin::contributePackagePlan), PackagePlanContribution>);
static_assert(!std::is_constructible_v<PackagePlan, std::string, std::string, std::string>);

TEST_CASE("PackagePlan owns typed package identity and location", "[domain][package-plan]") {
  const PackageLocation staging_location{.staging_path =
                                             std::filesystem::path{"/managed/packages/package-a"}};
  const PackagePlan staging_plan(PackageId{"package-a"}, staging_location, {});
  const PackagePlan staging_plan_copy(PackageId{"package-a"}, staging_location, {});

  REQUIRE(staging_plan == staging_plan_copy);
  REQUIRE(staging_plan.packageId() == PackageId{"package-a"});
  REQUIRE(staging_plan.packageLocation() == staging_location);
  REQUIRE(staging_plan.entries().empty());

  const PackageLocation object_location{.object_store_ref = "sha256:package-b"};
  const PackagePlan object_plan(PackageId{"package-b"}, object_location, {});

  REQUIRE(object_plan.packageLocation() == object_location);
  REQUIRE(object_plan != staging_plan);
}

TEST_CASE("PackagePlan preserves source entry kinds and order", "[domain][package-plan]") {
  std::string native_byte_path{"textures/"};
  native_byte_path.push_back(static_cast<char>(0xFF));
  native_byte_path += ".dds";

  const std::vector<PackageLayoutEntry> entries{
      PackageLayoutEntry{PackageRelativePath{"payload/loader.so"}, PackageEntryKind::RegularFile},
      PackageLayoutEntry{PackageRelativePath{"assets/Scripts"}, PackageEntryKind::Directory},
      PackageLayoutEntry{PackageRelativePath{"links/current"}, PackageEntryKind::SymbolicLink},
      PackageLayoutEntry{PackageRelativePath{native_byte_path}, PackageEntryKind::RegularFile},
  };
  const PackagePlan plan(
      PackageId{"package-layout"},
      PackageLocation{.staging_path = std::filesystem::path{"/managed/package-layout"}}, entries);

  REQUIRE(plan.entries() == entries);
  REQUIRE(plan.entries().at(0).relativePath() == PackageRelativePath{"payload/loader.so"});
  REQUIRE(plan.entries().at(0).kind() == PackageEntryKind::RegularFile);
  REQUIRE(plan.entries().at(1).kind() == PackageEntryKind::Directory);
  REQUIRE(plan.entries().at(2).kind() == PackageEntryKind::SymbolicLink);
  REQUIRE(plan.entries().at(3).relativePath().nativeBytes() == native_byte_path);
}
