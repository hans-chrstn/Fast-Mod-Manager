#include "domain/CompiledNamespace.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

static_assert(std::is_same_v<decltype(NamespaceCompilationResult::compile(
                                 std::declval<const ConflictResolutionReport&>())),
                             std::expected<NamespaceCompilationResult, NamespaceCompilationError>>);
static_assert(
    std::is_same_v<decltype(std::declval<const NamespaceCompilationResult&>().compiledNamespace()),
                   const CompiledNamespace&>);
static_assert(
    std::is_same_v<decltype(std::declval<const NamespaceCompilationResult&>().conflictReport()),
                   const ConflictResolutionReport&>);
static_assert(
    std::is_same_v<decltype(std::declval<const NamespaceCompilationResult&>().explanations()),
                   const std::vector<NamespaceEntryExplanation>&>);
static_assert(
    std::is_same_v<decltype(std::declval<const NamespaceEntryExplanation&>().contenders()),
                   const std::vector<PackagePathClaim>&>);
static_assert(!std::is_default_constructible_v<NamespaceCompilationResult>);
static_assert(!std::is_default_constructible_v<NamespaceEntryExplanation>);

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

auto decisionAt(const ConflictResolutionReport& conflict_report, const GameRelativePath& path)
    -> const ExactPathDecision& {
  const auto position = std::ranges::find(
      conflict_report.decisions(), path,
      [](const ExactPathDecision& decision) -> const GameRelativePath& { return decision.path(); });
  REQUIRE(position != conflict_report.decisions().end());
  return *position;
}

} // namespace

TEST_CASE("Compilation result explains every visible namespace entry from the same report",
          "[domain][namespace][explanation]") {
  const auto explicit_directory =
      claim("low", "source/explicit", "Explicit", PackageEntryKind::Directory);
  const auto winner = claim("high", "high/file", "Shared/file");
  const auto resolved = report({PackageId{"low"}, PackageId{"high"}},
                               {claim("low", "source/file", "Data/file"), explicit_directory,
                                claim("low", "low/merged", "Merged", PackageEntryKind::Directory),
                                claim("high", "high/merged", "Merged", PackageEntryKind::Directory),
                                claim("low", "low/file", "Shared/file"), winner},
                               {PackageId{"low"}, PackageId{"high"}});
  const auto result = NamespaceCompilationResult::compile(resolved);

  REQUIRE(result.has_value());
  REQUIRE(result->conflictReport() == resolved);
  REQUIRE(result->compiledNamespace().entries().size() == result->explanations().size());
  REQUIRE(result->explanations().size() == 6);
  for (std::size_t index = 0; index < result->explanations().size(); ++index) {
    REQUIRE(result->compiledNamespace().entries().at(index).path() ==
            result->explanations().at(index).path());
  }

  const auto& data_directory = result->explanations().at(0);
  REQUIRE(data_directory.path() == GameRelativePath{"Data"});
  REQUIRE(data_directory.kind() == NamespaceEntryExplanationKind::SyntheticAncestorDirectory);
  REQUIRE(data_directory.contenders().empty());
  REQUIRE_FALSE(data_directory.selectedClaim().has_value());

  const auto& file = result->explanations().at(1);
  const auto& file_decision = decisionAt(resolved, file.path());
  REQUIRE(file.kind() == NamespaceEntryExplanationKind::UncontestedClaim);
  REQUIRE(file.contenders() == file_decision.claims());
  REQUIRE(file.selectedClaim() == file_decision.selectedClaim());

  const auto& directory = result->explanations().at(2);
  REQUIRE(directory.kind() == NamespaceEntryExplanationKind::UncontestedClaim);
  REQUIRE(directory.selectedClaim() == explicit_directory);
  REQUIRE_FALSE(result->compiledNamespace().entries().at(2).selectedClaim().has_value());

  const auto& merged = result->explanations().at(3);
  const auto& merged_decision = decisionAt(resolved, merged.path());
  REQUIRE(merged.kind() == NamespaceEntryExplanationKind::MergedDirectory);
  REQUIRE(merged.contenders() == merged_decision.claims());
  REQUIRE_FALSE(merged.selectedClaim().has_value());

  const auto& shared_directory = result->explanations().at(4);
  REQUIRE(shared_directory.path() == GameRelativePath{"Shared"});
  REQUIRE(shared_directory.kind() == NamespaceEntryExplanationKind::SyntheticAncestorDirectory);
  REQUIRE(shared_directory.contenders().empty());
  REQUIRE_FALSE(shared_directory.selectedClaim().has_value());

  const auto& selected = result->explanations().at(5);
  const auto& selected_decision = decisionAt(resolved, selected.path());
  REQUIRE(selected.kind() == NamespaceEntryExplanationKind::PriorityWinner);
  REQUIRE(selected.contenders() == selected_decision.claims());
  REQUIRE(selected.selectedClaim() == winner);
  REQUIRE(selected.selectedClaim() == selected_decision.selectedClaim());
}

TEST_CASE("Compilation explanations own exact bytes and are deterministic across runs",
          "[domain][namespace][explanation]") {
  std::optional<NamespaceCompilationResult> owned_result;
  std::string source_bytes{"source/"};
  source_bytes.push_back(static_cast<char>(0xFE));
  std::string path_bytes;
  path_bytes.push_back(static_cast<char>(0xFF));
  path_bytes += "/file";
  const auto expected_source = source_bytes;
  const auto expected_path = path_bytes;

  {
    const auto resolved =
        report({PackageId{"package"}}, {claim("package", source_bytes, path_bytes)},
               {PackageId{"package"}});
    const auto first = NamespaceCompilationResult::compile(resolved);
    const auto second = NamespaceCompilationResult::compile(resolved);

    REQUIRE(first.has_value());
    REQUIRE(second.has_value());
    REQUIRE(first == second);
    owned_result.emplace(first.value());
  }

  source_bytes.clear();
  path_bytes.clear();
  REQUIRE(owned_result.has_value());
  REQUIRE(owned_result->explanations().size() == 2);
  REQUIRE(owned_result->explanations().at(0).kind() ==
          NamespaceEntryExplanationKind::SyntheticAncestorDirectory);
  REQUIRE(owned_result->explanations().at(1).path() == GameRelativePath{expected_path});
  REQUIRE(owned_result->explanations().at(1).contenders().size() == 1);
  REQUIRE(owned_result->explanations().at(1).contenders().front().packageRelativePath() ==
          PackageRelativePath{expected_source});
  REQUIRE(owned_result->explanations().at(1).selectedClaim() ==
          owned_result->explanations().at(1).contenders().front());
}

TEST_CASE("Empty reports produce one empty owning compilation result",
          "[domain][namespace][explanation]") {
  const auto empty_report = report({}, {}, {});
  const auto result = NamespaceCompilationResult::compile(empty_report);

  REQUIRE(result.has_value());
  REQUIRE(result->compiledNamespace().entries().empty());
  REQUIRE(result->explanations().empty());
  REQUIRE(result->conflictReport() == empty_report);
}

TEST_CASE("Blocking reports fail namespace and explanation creation together",
          "[domain][namespace][explanation]") {
  const auto blocked = report({PackageId{"a"}, PackageId{"b"}},
                              {claim("a", "a/directory", "mixed", PackageEntryKind::Directory),
                               claim("b", "b/file", "mixed")},
                              {PackageId{"a"}, PackageId{"b"}});
  const auto result = NamespaceCompilationResult::compile(blocked);

  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code() == NamespaceCompilationErrorCode::BlockingConflicts);
}
