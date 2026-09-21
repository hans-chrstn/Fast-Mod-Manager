#include "domain/ConflictResolution.hpp"

#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

static_assert(!std::is_same_v<ConflictPolicyError, ConflictDiagnostic>);
static_assert(!std::is_same_v<ExactPathDecision, FileOwnershipNode>);
static_assert(std::is_same_v<decltype(ConflictResolutionPolicy::create(
                                 std::declval<const FileOwnershipGraph&>(), {})),
                             std::expected<ConflictResolutionPolicy, ConflictPolicyError>>);
static_assert(std::is_same_v<decltype(ConflictResolutionReport::create(
                                 std::declval<const FileOwnershipGraph&>(), {})),
                             std::expected<ConflictResolutionReport, ConflictPolicyError>>);
static_assert(
    std::is_same_v<decltype(std::declval<const ConflictResolutionPolicy&>().priorityOrder()),
                   const std::vector<PackageId>&>);
static_assert(std::is_same_v<decltype(std::declval<const ExactPathDecision&>().claims()),
                             const std::vector<PackagePathClaim>&>);
static_assert(std::is_same_v<decltype(std::declval<const ExactPathDecision&>().selectedClaim()),
                             const std::optional<PackagePathClaim>&>);
static_assert(std::is_same_v<decltype(std::declval<const ConflictDiagnostic&>().relatedPath()),
                             const std::optional<GameRelativePath>&>);
static_assert(std::is_same_v<decltype(std::declval<const ConflictDiagnostic&>().claims()),
                             const std::vector<PackagePathClaim>&>);
static_assert(std::is_same_v<decltype(std::declval<const ConflictResolutionReport&>().policy()),
                             const ConflictResolutionPolicy&>);
static_assert(std::is_same_v<decltype(std::declval<const ConflictResolutionReport&>().decisions()),
                             const std::vector<ExactPathDecision>&>);
static_assert(
    std::is_same_v<decltype(std::declval<const ConflictResolutionReport&>().diagnostics()),
                   const std::vector<ConflictDiagnostic>&>);

namespace {

auto packageGraph(std::vector<PackageId> enabled, std::vector<PackageDependency> dependencies = {})
    -> PackageDependencyGraph {
  return PackageDependencyGraph::create(std::move(enabled), std::move(dependencies), {}).value();
}

auto claim(std::string package, std::string source, std::string destination,
           PackageEntryKind kind = PackageEntryKind::RegularFile) -> PackagePathClaim {
  return PackagePathClaim{PackageId{std::move(package)}, PackageRelativePath{std::move(source)},
                          GameRelativePath{std::move(destination)}, kind};
}

auto ownershipGraph(const PackageDependencyGraph& packages, std::vector<PackagePathClaim> claims)
    -> FileOwnershipGraph {
  return FileOwnershipGraph::create(packages, std::move(claims)).value();
}

auto requirePolicyError(std::expected<ConflictResolutionReport, ConflictPolicyError> result,
                        ConflictPolicyErrorCode code, const PackageId& package_id,
                        std::optional<std::size_t> priority_index) -> void {
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code() == code);
  REQUIRE(result.error().packageId() == package_id);
  REQUIRE(result.error().priorityIndex() == priority_index);
}

} // namespace

TEST_CASE("Conflict resolution supports empty graphs and claim-free packages",
          "[domain][conflict-resolution]") {
  const auto empty_graph = ownershipGraph(packageGraph({}), {});
  const auto empty_report = ConflictResolutionReport::create(empty_graph, {});

  REQUIRE(empty_report.has_value());
  REQUIRE(empty_report->policy().priorityOrder().empty());
  REQUIRE(empty_report->decisions().empty());
  REQUIRE(empty_report->diagnostics().empty());
  REQUIRE_FALSE(empty_report->hasBlockingConflicts());

  const auto packages = packageGraph({PackageId{"a"}, PackageId{"b"}});
  const auto claim_free_graph = ownershipGraph(packages, {});
  const auto claim_free_report =
      ConflictResolutionReport::create(claim_free_graph, {PackageId{"b"}, PackageId{"a"}});

  REQUIRE(claim_free_report.has_value());
  REQUIRE(claim_free_report->policy().priorityOrder() ==
          std::vector<PackageId>{PackageId{"b"}, PackageId{"a"}});
  REQUIRE(claim_free_report->decisions().empty());
  REQUIRE(claim_free_report->diagnostics().empty());
}

TEST_CASE("Conflict resolution owns policy and selects uncontested claims",
          "[domain][conflict-resolution]") {
  const auto packages = packageGraph({PackageId{"package"}});
  const auto graph = ownershipGraph(
      packages, {claim("package", "source/file", "file"),
                 claim("package", "source/directory", "directory", PackageEntryKind::Directory),
                 claim("package", "source/link", "link", PackageEntryKind::SymbolicLink)});
  std::vector<PackageId> priority{PackageId{"package"}};
  const auto report = ConflictResolutionReport::create(graph, priority);
  priority.clear();

  REQUIRE(report.has_value());
  REQUIRE(report->policy().priorityOrder() == std::vector<PackageId>{PackageId{"package"}});
  REQUIRE(report->decisions().size() == 3);
  for (const auto& decision : report->decisions()) {
    REQUIRE(decision.kind() == ExactPathDecisionKind::SingleClaim);
    REQUIRE(decision.claims().size() == 1);
    REQUIRE(decision.selectedClaim() == decision.claims().front());
  }
  REQUIRE(report->diagnostics().empty());
  REQUIRE_FALSE(report->hasBlockingConflicts());
}

TEST_CASE("Conflict resolution merges all-directory ownership without diagnostics",
          "[domain][conflict-resolution]") {
  const auto repeated = claim("a", "a/directory", "merged", PackageEntryKind::Directory);
  const auto graph = ownershipGraph(
      packageGraph({PackageId{"a"}, PackageId{"b"}}),
      {repeated, repeated, claim("b", "b/directory", "merged", PackageEntryKind::Directory)});
  const auto report = ConflictResolutionReport::create(graph, {PackageId{"b"}, PackageId{"a"}});

  REQUIRE(report.has_value());
  REQUIRE(report->decisions().size() == 1);
  REQUIRE(report->decisions().front().kind() == ExactPathDecisionKind::MergedDirectories);
  REQUIRE(report->decisions().front().claims() == graph.nodes().front().claims());
  REQUIRE_FALSE(report->decisions().front().selectedClaim().has_value());
  REQUIRE(report->diagnostics().empty());
  REQUIRE_FALSE(report->hasBlockingConflicts());
}

TEST_CASE("Explicit conflict priority is independent of dependency and incoming claim order",
          "[domain][conflict-resolution]") {
  const auto packages =
      packageGraph({PackageId{"dependent"}, PackageId{"required"}},
                   {PackageDependency{PackageId{"dependent"}, PackageId{"required"}}});
  const auto graph = ownershipGraph(packages, {claim("dependent", "dependent/source", "shared"),
                                               claim("required", "required/source", "shared")});
  const auto reversed_graph =
      ownershipGraph(packages, {claim("required", "required/source", "shared"),
                                claim("dependent", "dependent/source", "shared")});
  const auto report =
      ConflictResolutionReport::create(graph, {PackageId{"dependent"}, PackageId{"required"}});
  const auto reversed_report = ConflictResolutionReport::create(
      reversed_graph, {PackageId{"dependent"}, PackageId{"required"}});

  REQUIRE(graph.packageOrder() ==
          std::vector<PackageId>{PackageId{"required"}, PackageId{"dependent"}});
  REQUIRE(report.has_value());
  REQUIRE(reversed_report.has_value());
  REQUIRE(report == reversed_report);
  REQUIRE(report->decisions().front().kind() == ExactPathDecisionKind::PriorityWinner);
  REQUIRE(report->decisions().front().selectedClaim() ==
          claim("required", "required/source", "shared"));
  REQUIRE(report->diagnostics().size() == 1);
  REQUIRE(report->diagnostics().front().selectedClaim() ==
          claim("required", "required/source", "shared"));
}

TEST_CASE("Conflict resolution selects winners for every non-directory kind pairing",
          "[domain][conflict-resolution]") {
  const auto packages = packageGraph({PackageId{"low"}, PackageId{"high"}});

  const auto requireWinner = [&packages](PackageEntryKind low_kind,
                                         PackageEntryKind high_kind) -> void {
    const auto low_claim = claim("low", "low/source", "shared", low_kind);
    const auto high_claim = claim("high", "high/source", "shared", high_kind);
    const auto graph = ownershipGraph(packages, {high_claim, low_claim});
    const auto report =
        ConflictResolutionReport::create(graph, {PackageId{"low"}, PackageId{"high"}});

    REQUIRE(report.has_value());
    REQUIRE(report->decisions().front().kind() == ExactPathDecisionKind::PriorityWinner);
    REQUIRE(report->decisions().front().selectedClaim() == high_claim);
    REQUIRE(report->diagnostics().size() == 1);
    const auto& diagnostic = report->diagnostics().front();
    REQUIRE(diagnostic.code() == ConflictDiagnosticCode::ExactPathWinnerSelected);
    REQUIRE(diagnostic.severity() == ConflictDiagnosticSeverity::Warning);
    REQUIRE(diagnostic.primaryPath() == GameRelativePath{"shared"});
    REQUIRE_FALSE(diagnostic.relatedPath().has_value());
    REQUIRE(diagnostic.claims() == graph.nodes().front().claims());
    REQUIRE(diagnostic.selectedClaim() == high_claim);
    REQUIRE(diagnostic.remediation() == ConflictRemediation::ReviewPackagePriority);
    REQUIRE_FALSE(report->hasBlockingConflicts());
  };

  SECTION("file overrides file") {
    requireWinner(PackageEntryKind::RegularFile, PackageEntryKind::RegularFile);
  }
  SECTION("symbolic link overrides file") {
    requireWinner(PackageEntryKind::RegularFile, PackageEntryKind::SymbolicLink);
  }
  SECTION("file overrides symbolic link") {
    requireWinner(PackageEntryKind::SymbolicLink, PackageEntryKind::RegularFile);
  }
  SECTION("symbolic link overrides symbolic link") {
    requireWinner(PackageEntryKind::SymbolicLink, PackageEntryKind::SymbolicLink);
  }
}

TEST_CASE("Repeated losing claims do not change a unique higher-priority winner",
          "[domain][conflict-resolution]") {
  const auto repeated = claim("low", "low/repeated", "shared");
  const auto winner = claim("high", "high/source", "shared");
  const auto graph = ownershipGraph(packageGraph({PackageId{"low"}, PackageId{"high"}}),
                                    {repeated, winner, repeated});
  const auto report =
      ConflictResolutionReport::create(graph, {PackageId{"low"}, PackageId{"high"}});

  REQUIRE(report.has_value());
  REQUIRE(report->decisions().front().selectedClaim() == winner);
  REQUIRE(report->decisions().front().claims().size() == 3);
  REQUIRE(report->diagnostics().front().claims().size() == 3);
  REQUIRE_FALSE(report->hasBlockingConflicts());
}

TEST_CASE("Mixed directory and non-directory claims are blocking at every priority",
          "[domain][conflict-resolution]") {
  const auto packages = packageGraph({PackageId{"a"}, PackageId{"b"}});

  const auto requireMixedError = [&packages](PackageEntryKind non_directory_kind,
                                             std::vector<PackageId> priority) -> void {
    const auto graph =
        ownershipGraph(packages, {claim("a", "a/directory", "mixed", PackageEntryKind::Directory),
                                  claim("b", "b/entry", "mixed", non_directory_kind)});
    const auto report = ConflictResolutionReport::create(graph, std::move(priority));

    REQUIRE(report.has_value());
    REQUIRE(report->decisions().front().kind() == ExactPathDecisionKind::Unresolved);
    REQUIRE_FALSE(report->decisions().front().selectedClaim().has_value());
    REQUIRE(report->diagnostics().size() == 1);
    REQUIRE(report->diagnostics().front().code() == ConflictDiagnosticCode::MixedEntryKinds);
    REQUIRE(report->diagnostics().front().severity() == ConflictDiagnosticSeverity::Error);
    REQUIRE(report->diagnostics().front().remediation() ==
            ConflictRemediation::CorrectPackageMapping);
    REQUIRE(report->hasBlockingConflicts());
  };

  SECTION("directory lower than file") {
    requireMixedError(PackageEntryKind::RegularFile, {PackageId{"a"}, PackageId{"b"}});
  }
  SECTION("directory higher than file") {
    requireMixedError(PackageEntryKind::RegularFile, {PackageId{"b"}, PackageId{"a"}});
  }
  SECTION("directory and symbolic link") {
    requireMixedError(PackageEntryKind::SymbolicLink, {PackageId{"a"}, PackageId{"b"}});
  }
}

TEST_CASE("Multiple highest-priority package claims are blocking ambiguities",
          "[domain][conflict-resolution]") {
  const auto packages = packageGraph({PackageId{"low"}, PackageId{"high"}});

  const auto requireAmbiguity = [&packages](const PackagePathClaim& first_high,
                                            const PackagePathClaim& second_high) -> void {
    const auto graph =
        ownershipGraph(packages, {claim("low", "low/source", "shared"), first_high, second_high});
    const auto report =
        ConflictResolutionReport::create(graph, {PackageId{"low"}, PackageId{"high"}});

    REQUIRE(report.has_value());
    REQUIRE(report->decisions().front().kind() == ExactPathDecisionKind::Unresolved);
    REQUIRE_FALSE(report->decisions().front().selectedClaim().has_value());
    REQUIRE(report->diagnostics().front().code() ==
            ConflictDiagnosticCode::AmbiguousHighestPriorityClaim);
    REQUIRE(report->diagnostics().front().severity() == ConflictDiagnosticSeverity::Error);
    REQUIRE(report->diagnostics().front().remediation() ==
            ConflictRemediation::CorrectPackageMapping);
    REQUIRE(report->hasBlockingConflicts());
  };

  const auto repeated = claim("high", "high/repeated", "shared");
  SECTION("repeated identical claims") { requireAmbiguity(repeated, repeated); }
  SECTION("distinct source claims") {
    requireAmbiguity(claim("high", "high/first", "shared"),
                     claim("high", "high/second", "shared", PackageEntryKind::SymbolicLink));
  }
}

TEST_CASE("File and symbolic-link ancestor obstructions are blocking structured diagnostics",
          "[domain][conflict-resolution]") {
  const auto packages = packageGraph({PackageId{"package"}});
  const auto graph = ownershipGraph(
      packages, {claim("package", "source/file", "Data/file"),
                 claim("package", "source/child", "Data/file/child"),
                 claim("package", "source/grandchild", "Data/file/child/grandchild"),
                 claim("package", "source/link", "Links/current", PackageEntryKind::SymbolicLink),
                 claim("package", "source/link-child", "Links/current/item")});
  const auto report = ConflictResolutionReport::create(graph, {PackageId{"package"}});

  REQUIRE(report.has_value());
  REQUIRE(report->diagnostics().size() == 4);
  REQUIRE(report->hasBlockingConflicts());

  const auto& file_obstruction = report->diagnostics().at(0);
  REQUIRE(file_obstruction.code() == ConflictDiagnosticCode::PathObstruction);
  REQUIRE(file_obstruction.severity() == ConflictDiagnosticSeverity::Error);
  REQUIRE(file_obstruction.primaryPath() == GameRelativePath{"Data/file"});
  REQUIRE(file_obstruction.relatedPath() == GameRelativePath{"Data/file/child"});
  REQUIRE(file_obstruction.claims() ==
          std::vector<PackagePathClaim>{claim("package", "source/file", "Data/file"),
                                        claim("package", "source/child", "Data/file/child")});
  REQUIRE_FALSE(file_obstruction.selectedClaim().has_value());
  REQUIRE(file_obstruction.remediation() == ConflictRemediation::CorrectPackageLayout);

  const auto& nested_from_root = report->diagnostics().at(1);
  REQUIRE(nested_from_root.primaryPath() == GameRelativePath{"Data/file"});
  REQUIRE(nested_from_root.relatedPath() == GameRelativePath{"Data/file/child/grandchild"});

  const auto& nested_from_child = report->diagnostics().at(2);
  REQUIRE(nested_from_child.primaryPath() == GameRelativePath{"Data/file/child"});
  REQUIRE(nested_from_child.relatedPath() == GameRelativePath{"Data/file/child/grandchild"});

  const auto& link_obstruction = report->diagnostics().at(3);
  REQUIRE(link_obstruction.primaryPath() == GameRelativePath{"Links/current"});
  REQUIRE(link_obstruction.relatedPath() == GameRelativePath{"Links/current/item"});
}

TEST_CASE("Exact diagnostics precede obstruction diagnostics deterministically",
          "[domain][conflict-resolution]") {
  const auto packages = packageGraph({PackageId{"low"}, PackageId{"high"}});
  const std::vector<PackagePathClaim> claims{
      claim("high", "high/exact", "a/exact"),
      claim("low", "low/exact", "a/exact"),
      claim("low", "low/mixed", "b/mixed", PackageEntryKind::Directory),
      claim("high", "high/mixed", "b/mixed"),
      claim("low", "low/ancestor", "z/file"),
      claim("high", "high/descendant", "z/file/child"),
  };
  const auto graph = ownershipGraph(packages, claims);
  const auto first = ConflictResolutionReport::create(graph, {PackageId{"low"}, PackageId{"high"}});
  const auto second =
      ConflictResolutionReport::create(graph, {PackageId{"low"}, PackageId{"high"}});

  REQUIRE(first.has_value());
  REQUIRE(second.has_value());
  REQUIRE(first == second);
  REQUIRE(first->diagnostics().size() == 3);
  REQUIRE(first->diagnostics().at(0).code() == ConflictDiagnosticCode::ExactPathWinnerSelected);
  REQUIRE(first->diagnostics().at(0).primaryPath() == GameRelativePath{"a/exact"});
  REQUIRE(first->diagnostics().at(1).code() == ConflictDiagnosticCode::MixedEntryKinds);
  REQUIRE(first->diagnostics().at(1).primaryPath() == GameRelativePath{"b/mixed"});
  REQUIRE(first->diagnostics().at(2).code() == ConflictDiagnosticCode::PathObstruction);
  REQUIRE(first->diagnostics().at(2).primaryPath() == GameRelativePath{"z/file"});
  REQUIRE(first->diagnostics().at(2).relatedPath() == GameRelativePath{"z/file/child"});
}

TEST_CASE("Conflict policy rejects duplicate unknown and missing packages deterministically",
          "[domain][conflict-resolution]") {
  const auto graph =
      ownershipGraph(packageGraph({PackageId{"a"}, PackageId{"b"}, PackageId{"c"}}), {});

  requirePolicyError(ConflictResolutionReport::create(
                         graph, {PackageId{"a"}, PackageId{"a"}, PackageId{"b"}, PackageId{"c"}}),
                     ConflictPolicyErrorCode::DuplicatePackage, PackageId{"a"}, 1);
  requirePolicyError(ConflictResolutionReport::create(graph, {PackageId{"a"}, PackageId{"unknown"},
                                                              PackageId{"b"}, PackageId{"c"}}),
                     ConflictPolicyErrorCode::UnknownPackage, PackageId{"unknown"}, 1);
  requirePolicyError(ConflictResolutionReport::create(graph, {PackageId{"b"}, PackageId{"c"}}),
                     ConflictPolicyErrorCode::MissingPackage, PackageId{"a"}, std::nullopt);
  requirePolicyError(ConflictResolutionReport::create(graph, {PackageId{"a"}, PackageId{"c"}}),
                     ConflictPolicyErrorCode::MissingPackage, PackageId{"b"}, std::nullopt);
  requirePolicyError(ConflictResolutionReport::create(graph, {PackageId{"a"}, PackageId{"b"}}),
                     ConflictPolicyErrorCode::MissingPackage, PackageId{"c"}, std::nullopt);
  requirePolicyError(
      ConflictResolutionReport::create(graph, {PackageId{"a"}, PackageId{"unknown"}, PackageId{"a"},
                                               PackageId{"b"}, PackageId{"c"}}),
      ConflictPolicyErrorCode::UnknownPackage, PackageId{"unknown"}, 1);

  const auto duplicate_policy = ConflictResolutionPolicy::create(
      graph, {PackageId{"a"}, PackageId{"a"}, PackageId{"b"}, PackageId{"c"}});
  REQUIRE_FALSE(duplicate_policy.has_value());
  REQUIRE(duplicate_policy.error().code() == ConflictPolicyErrorCode::DuplicatePackage);
  REQUIRE(duplicate_policy.error().priorityIndex() == 1);
}

TEST_CASE("Conflict diagnostics preserve empty roots and arbitrary non-NUL path bytes",
          "[domain][conflict-resolution]") {
  std::string high_byte_path;
  high_byte_path.push_back(static_cast<char>(0xFF));
  high_byte_path += "/shared";
  std::string high_byte_source{"source/"};
  high_byte_source.push_back(static_cast<char>(0xFE));

  const auto packages = packageGraph({PackageId{"low"}, PackageId{"high"}});
  const auto high_byte_winner = claim("high", high_byte_source, high_byte_path);
  const auto graph = ownershipGraph(
      packages, {claim("low", "root/source", ""), claim("high", "child/source", "child"),
                 claim("low", "low/bytes", high_byte_path), high_byte_winner});
  std::vector<PackageId> priority{PackageId{"low"}, PackageId{"high"}};
  const auto report = ConflictResolutionReport::create(graph, priority);
  priority.clear();

  REQUIRE(report.has_value());
  REQUIRE(report->policy().priorityOrder() ==
          std::vector<PackageId>{PackageId{"low"}, PackageId{"high"}});
  REQUIRE(report->diagnostics().size() == 3);
  REQUIRE(report->diagnostics().at(0).code() == ConflictDiagnosticCode::ExactPathWinnerSelected);
  REQUIRE(report->diagnostics().at(0).primaryPath() == GameRelativePath{high_byte_path});
  REQUIRE(report->diagnostics().at(0).selectedClaim() == high_byte_winner);
  REQUIRE(report->diagnostics().at(0).selectedClaim().transform(
              [](const PackagePathClaim& selected) { return selected.packageRelativePath(); }) ==
          std::optional<PackageRelativePath>{PackageRelativePath{high_byte_source}});
  REQUIRE(report->diagnostics().at(1).code() == ConflictDiagnosticCode::PathObstruction);
  REQUIRE(report->diagnostics().at(1).primaryPath() == GameRelativePath{""});
  REQUIRE(report->diagnostics().at(1).relatedPath() == GameRelativePath{"child"});
  REQUIRE(report->diagnostics().at(2).primaryPath() == GameRelativePath{""});
  REQUIRE(report->diagnostics().at(2).relatedPath() == GameRelativePath{high_byte_path});
  REQUIRE(report->hasBlockingConflicts());
}
