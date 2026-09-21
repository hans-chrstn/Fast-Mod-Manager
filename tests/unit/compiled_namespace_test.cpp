#include "domain/CompiledNamespace.hpp"

#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

static_assert(std::is_same_v<
              decltype(CompiledNamespace::compile(std::declval<const ConflictResolutionReport&>())),
              std::expected<CompiledNamespace, NamespaceCompilationError>>);
static_assert(std::is_same_v<decltype(std::declval<const CompiledNamespace&>().entries()),
                             const std::vector<CompiledNamespaceEntry>&>);
static_assert(
    std::is_same_v<decltype(std::declval<const CompiledNamespaceEntry&>().selectedClaim()),
                   const std::optional<PackagePathClaim>&>);
static_assert(!std::is_default_constructible_v<CompiledNamespace>);
static_assert(!std::is_default_constructible_v<CompiledNamespaceEntry>);

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

TEST_CASE("Empty conflict reports compile to empty namespaces", "[domain][namespace]") {
  const auto empty_report = report({}, {}, {});
  const auto compiled = CompiledNamespace::compile(empty_report);

  REQUIRE(compiled.has_value());
  REQUIRE(compiled->entries().empty());
}

TEST_CASE("Resolved decisions compile without reinterpreting winners", "[domain][namespace]") {
  const auto winner = claim("high", "high/source", "shared");
  const auto resolved =
      report({PackageId{"low"}, PackageId{"high"}},
             {claim("low", "Data/file", "Data/file"),
              claim("low", "Explicit", "Explicit", PackageEntryKind::Directory),
              claim("low", "Links/current", "Links/current", PackageEntryKind::SymbolicLink),
              claim("low", "low/merged", "Merged", PackageEntryKind::Directory),
              claim("high", "high/merged", "Merged", PackageEntryKind::Directory),
              claim("low", "low/source", "shared"), winner},
             {PackageId{"low"}, PackageId{"high"}});
  const auto compiled = CompiledNamespace::compile(resolved);

  REQUIRE(compiled.has_value());
  REQUIRE(compiled->entries().size() == 7);

  const auto& data = compiled->entries().at(0);
  REQUIRE(data.path() == GameRelativePath{"Data"});
  REQUIRE(data.kind() == PackageEntryKind::Directory);
  REQUIRE(data.origin() == CompiledNamespaceEntryOrigin::SyntheticDirectory);
  REQUIRE_FALSE(data.selectedClaim().has_value());

  const auto& file = compiled->entries().at(1);
  REQUIRE(file.path() == GameRelativePath{"Data/file"});
  REQUIRE(file.kind() == PackageEntryKind::RegularFile);
  REQUIRE(file.origin() == CompiledNamespaceEntryOrigin::SingleClaim);
  REQUIRE(file.selectedClaim() == claim("low", "Data/file", "Data/file"));

  const auto& explicit_directory = compiled->entries().at(2);
  REQUIRE(explicit_directory.path() == GameRelativePath{"Explicit"});
  REQUIRE(explicit_directory.kind() == PackageEntryKind::Directory);
  REQUIRE(explicit_directory.origin() == CompiledNamespaceEntryOrigin::SingleClaim);
  REQUIRE_FALSE(explicit_directory.selectedClaim().has_value());

  REQUIRE(compiled->entries().at(3).path() == GameRelativePath{"Links"});
  REQUIRE(compiled->entries().at(3).origin() == CompiledNamespaceEntryOrigin::SyntheticDirectory);
  REQUIRE(compiled->entries().at(4).kind() == PackageEntryKind::SymbolicLink);
  REQUIRE(compiled->entries().at(4).selectedClaim() ==
          claim("low", "Links/current", "Links/current", PackageEntryKind::SymbolicLink));

  const auto& merged = compiled->entries().at(5);
  REQUIRE(merged.path() == GameRelativePath{"Merged"});
  REQUIRE(merged.kind() == PackageEntryKind::Directory);
  REQUIRE(merged.origin() == CompiledNamespaceEntryOrigin::MergedDirectories);
  REQUIRE_FALSE(merged.selectedClaim().has_value());

  const auto& selected = compiled->entries().at(6);
  REQUIRE(selected.path() == GameRelativePath{"shared"});
  REQUIRE(selected.origin() == CompiledNamespaceEntryOrigin::PriorityWinner);
  REQUIRE(selected.selectedClaim() == winner);
}

TEST_CASE("Namespace compilation synthesizes each missing ancestor exactly once",
          "[domain][namespace]") {
  const auto resolved = report(
      {PackageId{"package"}},
      {claim("package", "root", "", PackageEntryKind::Directory),
       claim("package", "source/explicit", "a/b", PackageEntryKind::Directory),
       claim("package", "source/first", "a/b/c"), claim("package", "source/second", "a/d/e")},
      {PackageId{"package"}});
  const auto compiled = CompiledNamespace::compile(resolved);

  REQUIRE(compiled.has_value());
  REQUIRE(compiled->entries().size() == 6);
  REQUIRE(compiled->entries().at(0).path() == GameRelativePath{""});
  REQUIRE(compiled->entries().at(0).origin() == CompiledNamespaceEntryOrigin::SingleClaim);
  REQUIRE(compiled->entries().at(1).path() == GameRelativePath{"a"});
  REQUIRE(compiled->entries().at(1).origin() == CompiledNamespaceEntryOrigin::SyntheticDirectory);
  REQUIRE(compiled->entries().at(2).path() == GameRelativePath{"a/b"});
  REQUIRE(compiled->entries().at(2).origin() == CompiledNamespaceEntryOrigin::SingleClaim);
  REQUIRE(compiled->entries().at(3).path() == GameRelativePath{"a/b/c"});
  REQUIRE(compiled->entries().at(4).path() == GameRelativePath{"a/d"});
  REQUIRE(compiled->entries().at(4).origin() == CompiledNamespaceEntryOrigin::SyntheticDirectory);
  REQUIRE(compiled->entries().at(5).path() == GameRelativePath{"a/d/e"});
}

TEST_CASE("Namespace output owns claims and uses exact unsigned byte order",
          "[domain][namespace]") {
  std::string source_bytes{"source/"};
  source_bytes.push_back(static_cast<char>(0xFE));
  std::string path_bytes;
  path_bytes.push_back(static_cast<char>(0xFF));
  path_bytes += "/file";

  const auto expected_source = source_bytes;
  const auto expected_path = path_bytes;
  const auto resolved =
      report({PackageId{"package"}},
             {claim("package", source_bytes, path_bytes), claim("package", "source/a", "a/file"),
              claim("package", "source/z", "z")},
             {PackageId{"package"}});
  source_bytes.clear();
  path_bytes.clear();

  const auto first = CompiledNamespace::compile(resolved);
  const auto second = CompiledNamespace::compile(resolved);

  REQUIRE(first.has_value());
  REQUIRE(second.has_value());
  REQUIRE(first == second);
  REQUIRE(first->entries().size() == 5);
  REQUIRE(first->entries().at(0).path() == GameRelativePath{"a"});
  REQUIRE(first->entries().at(1).path() == GameRelativePath{"a/file"});
  REQUIRE(first->entries().at(2).path() == GameRelativePath{"z"});
  REQUIRE(first->entries().at(3).path() ==
          GameRelativePath{expected_path.substr(0, expected_path.find('/'))});
  REQUIRE(first->entries().at(4).path() == GameRelativePath{expected_path});
  REQUIRE(first->entries().at(4).selectedClaim().transform([](const PackagePathClaim& selected) {
    return selected.packageRelativePath();
  }) == std::optional<PackageRelativePath>{PackageRelativePath{expected_source}});
}

TEST_CASE("Blocking conflict reports return typed failure without a namespace",
          "[domain][namespace]") {
  SECTION("mixed entry kinds") {
    const auto blocked = report({PackageId{"a"}, PackageId{"b"}},
                                {claim("a", "a/directory", "mixed", PackageEntryKind::Directory),
                                 claim("b", "b/file", "mixed")},
                                {PackageId{"a"}, PackageId{"b"}});
    const auto compiled = CompiledNamespace::compile(blocked);

    REQUIRE_FALSE(compiled.has_value());
    REQUIRE(compiled.error().code() == NamespaceCompilationErrorCode::BlockingConflicts);
  }

  SECTION("ambiguous highest-priority claim") {
    const auto blocked =
        report({PackageId{"low"}, PackageId{"high"}},
               {claim("low", "low/file", "shared"), claim("high", "high/first", "shared"),
                claim("high", "high/second", "shared", PackageEntryKind::SymbolicLink)},
               {PackageId{"low"}, PackageId{"high"}});
    const auto compiled = CompiledNamespace::compile(blocked);

    REQUIRE_FALSE(compiled.has_value());
    REQUIRE(compiled.error().code() == NamespaceCompilationErrorCode::BlockingConflicts);
  }

  SECTION("non-directory ancestor obstruction") {
    const auto blocked = report({PackageId{"package"}},
                                {claim("package", "source/file", "Data/file"),
                                 claim("package", "source/child", "Data/file/child")},
                                {PackageId{"package"}});
    const auto compiled = CompiledNamespace::compile(blocked);

    REQUIRE_FALSE(compiled.has_value());
    REQUIRE(compiled.error().code() == NamespaceCompilationErrorCode::BlockingConflicts);
  }
}
