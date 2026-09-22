#include "domain/GameFsPlan.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

template <typename Plan>
concept HasFlatDeployedFiles = requires(Plan plan) { plan.deployed_files; };

template <typename Plan>
concept HasRawTargetGameDirectory = requires(Plan plan) { plan.target_game_directory; };

template <typename Plan>
concept HasRawProfileId = requires(Plan plan) { plan.profile_id; };

static_assert(!HasFlatDeployedFiles<GameFsPlan>);
static_assert(!HasRawTargetGameDirectory<GameFsPlan>);
static_assert(!HasRawProfileId<GameFsPlan>);
static_assert(!std::is_default_constructible_v<GameFsPlan>);
static_assert(std::is_same_v<decltype(std::declval<const GameFsPlan&>().profileIdentity()),
                             const ProfileIdentity&>);
static_assert(std::is_same_v<decltype(std::declval<const GameFsPlan&>().namespaceResult()),
                             const NamespaceCompilationResult&>);

namespace {

auto packageGraph(std::vector<PackageId> enabled) -> PackageDependencyGraph {
  return PackageDependencyGraph::create(std::move(enabled), {}, {}).value();
}

auto claim(std::string package, std::string source, std::string destination,
           PackageEntryKind kind = PackageEntryKind::RegularFile) -> PackagePathClaim {
  return PackagePathClaim{PackageId{std::move(package)}, PackageRelativePath{std::move(source)},
                          GameRelativePath{std::move(destination)}, kind};
}

auto report(std::vector<PackageId> packages, std::vector<PackagePathClaim> claims,
            std::vector<PackageId> priority) -> ConflictResolutionReport {
  const auto graph =
      FileOwnershipGraph::create(packageGraph(std::move(packages)), std::move(claims)).value();
  return ConflictResolutionReport::create(graph, std::move(priority)).value();
}

} // namespace

TEST_CASE("GameFsPlan owns typed empty profile and namespace values", "[domain][gamefs-plan]") {
  const auto profile = ProfileIdentity::create("Empty Profile").value();
  const auto compilation = NamespaceCompilationResult::compile(report({}, {}, {}));

  REQUIRE(compilation.has_value());
  const GameFsPlan plan{profile, compilation.value()};
  REQUIRE(plan.profileIdentity() == profile);
  REQUIRE(plan.namespaceResult() == compilation.value());
  REQUIRE(plan.namespaceResult().compiledNamespace().entries().empty());
  REQUIRE(plan.namespaceResult().explanations().empty());
}

TEST_CASE("GameFsPlan preserves owned compilation data and deterministic equality",
          "[domain][gamefs-plan]") {
  std::string profile_name{"Owned Profile"};
  std::string package_id{"package"};
  std::string source_bytes{"source/"};
  source_bytes.push_back(static_cast<char>(0xFE));
  std::string path_bytes{"Data/"};
  path_bytes.push_back(static_cast<char>(0xFF));
  const auto expected_profile = profile_name;
  const auto expected_package = package_id;
  const auto expected_source = source_bytes;
  const auto expected_path = path_bytes;

  const auto profile = ProfileIdentity::create(profile_name).value();
  const auto resolved =
      report({PackageId{package_id}}, {claim(package_id, source_bytes, path_bytes)},
             {PackageId{package_id}});
  const auto compilation = NamespaceCompilationResult::compile(resolved);

  REQUIRE(compilation.has_value());
  const GameFsPlan first{profile, compilation.value()};
  const GameFsPlan second{profile, compilation.value()};
  profile_name.clear();
  package_id.clear();
  source_bytes.clear();
  path_bytes.clear();

  REQUIRE(first == second);
  REQUIRE(first.profileIdentity().name() == expected_profile);
  REQUIRE(first.namespaceResult().compiledNamespace().entries().size() == 2);
  REQUIRE(first.namespaceResult().explanations().size() == 2);
  const auto& visible = first.namespaceResult().compiledNamespace().entries().at(1);
  REQUIRE(visible.path() == GameRelativePath{expected_path});
  REQUIRE(visible.selectedClaim() == claim(expected_package, expected_source, expected_path));
  REQUIRE(first.namespaceResult().explanations().at(1).selectedClaim() == visible.selectedClaim());
}

TEST_CASE("Blocking conflicts cannot become GameFsPlan namespace input", "[domain][gamefs-plan]") {
  const auto blocked = report({PackageId{"a"}, PackageId{"b"}},
                              {claim("a", "a/directory", "mixed", PackageEntryKind::Directory),
                               claim("b", "b/file", "mixed")},
                              {PackageId{"a"}, PackageId{"b"}});
  const auto compilation = NamespaceCompilationResult::compile(blocked);

  REQUIRE_FALSE(compilation.has_value());
  REQUIRE(compilation.error().code() == NamespaceCompilationErrorCode::BlockingConflicts);
}
