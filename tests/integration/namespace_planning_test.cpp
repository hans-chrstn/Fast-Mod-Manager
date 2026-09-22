#include "domain/GameFsPlan.hpp"
#include "fixtures/namespace/NamespacePlanningFixtures.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace fmm::domain;
namespace fixtures = fmm::test::namespace_fixtures;

namespace {

enum class FixtureMappingErrorCode : std::uint8_t { MissingMapping, AmbiguousMapping };

struct FixtureMappingError final {
  FixtureMappingErrorCode code{FixtureMappingErrorCode::MissingMapping};
  PackageId package_id;
  PackageRelativePath package_path;
};

struct ResolvedFixture final {
  std::vector<NamespacePackagePlanRecord> package_snapshot;
  PackageDependencyGraph package_graph;
  FileOwnershipGraph ownership_graph;
  ConflictResolutionReport conflict_report;
};

auto mapClaims(const std::vector<PackagePlan>& package_plans)
    -> std::expected<std::vector<PackagePathClaim>, FixtureMappingError> {
  std::vector<PackagePathClaim> claims;
  for (const auto& plan : package_plans) {
    for (const auto& entry : plan.entries()) {
      std::optional<GameRelativePath> mapped_path;
      for (const auto& mapping : plan.rootMappings()) {
        auto candidate = mapping.map(entry.relativePath());
        if (!candidate.has_value()) {
          continue;
        }
        if (mapped_path.has_value()) {
          return std::unexpected(
              FixtureMappingError{.code = FixtureMappingErrorCode::AmbiguousMapping,
                                  .package_id = plan.packageId(),
                                  .package_path = entry.relativePath()});
        }
        mapped_path = std::move(candidate).value();
      }
      if (!mapped_path.has_value()) {
        return std::unexpected(FixtureMappingError{
            .code = FixtureMappingErrorCode::MissingMapping,
            .package_id = plan.packageId(),
            .package_path = entry.relativePath(),
        });
      }
      claims.emplace_back(plan.packageId(), entry.relativePath(), std::move(mapped_path).value(),
                          entry.kind());
    }
  }
  return claims;
}

auto packageSnapshot(const std::vector<PackagePlan>& package_plans)
    -> std::vector<NamespacePackagePlanRecord> {
  std::vector<NamespacePackagePlanRecord> records;
  records.reserve(package_plans.size());
  std::ranges::transform(package_plans, std::back_inserter(records),
                         [](const PackagePlan& plan) -> NamespacePackagePlanRecord {
                           return NamespacePackagePlanRecord{plan.packageId(), plan};
                         });
  return records;
}

auto resolveFixture(const fixtures::PlanningFixture& fixture) -> ResolvedFixture {
  auto claims = mapClaims(fixture.package_plans);
  REQUIRE(claims.has_value());

  auto package_graph = PackageDependencyGraph::create(
      fixture.enabled_packages, fixture.dependencies, fixture.ordering_constraints);
  REQUIRE(package_graph.has_value());

  auto ownership = FileOwnershipGraph::create(*package_graph, std::move(*claims));
  REQUIRE(ownership.has_value());

  auto report = ConflictResolutionReport::create(*ownership, fixture.priority_order);
  REQUIRE(report.has_value());

  return ResolvedFixture{
      .package_snapshot = packageSnapshot(fixture.package_plans),
      .package_graph = std::move(*package_graph),
      .ownership_graph = std::move(*ownership),
      .conflict_report = std::move(*report),
  };
}

auto entryAt(const SourceMappedNamespace& mapped_namespace, const GameRelativePath& path)
    -> const CompiledNamespaceEntry& {
  const auto position = std::ranges::find_if(
      mapped_namespace.entries(),
      [&path](const CompiledNamespaceEntry& entry) -> bool { return entry.path() == path; });
  const auto index =
      static_cast<std::size_t>(std::ranges::distance(mapped_namespace.entries().begin(), position));
  REQUIRE(index < mapped_namespace.entries().size());
  return mapped_namespace.entries().at(index);
}

auto hasDiagnostic(const ConflictResolutionReport& report, ConflictDiagnosticCode code) -> bool {
  return std::ranges::any_of(report.diagnostics(), [code](const ConflictDiagnostic& diagnostic) {
    return diagnostic.code() == code;
  });
}

} // namespace

TEST_CASE("Synthetic fixture spans package planning through GameFsPlan",
          "[integration][namespace]") {
  auto fixture = fixtures::ordinaryFixture();
  const auto arbitrary_path = fixture.package_plans.at(1).entries().back().relativePath();
  const auto resolved = resolveFixture(fixture);

  REQUIRE(resolved.package_graph.resolvedOrder() ==
          std::vector<PackageId>{PackageId{"low"}, PackageId{"high"}});
  REQUIRE_FALSE(resolved.conflict_report.hasBlockingConflicts());
  REQUIRE(resolved.conflict_report.diagnostics().size() == 2);

  const auto compilation = NamespaceCompilationResult::compile(resolved.conflict_report);
  REQUIRE(compilation.has_value());
  const auto profile = ProfileIdentity::create(fixture.profile_name);
  REQUIRE(profile.has_value());
  const GameFsPlan plan{*profile, *compilation};
  const auto mapped =
      plan.namespaceResult().compiledNamespace().mapSources(resolved.package_snapshot);

  REQUIRE(mapped.has_value());
  REQUIRE(mapped->entries().size() == 12);
  REQUIRE(plan.namespaceResult().explanations().size() == mapped->entries().size());
  for (std::size_t index = 0; index < mapped->entries().size(); ++index) {
    REQUIRE(plan.namespaceResult().explanations().at(index).path() ==
            mapped->entries().at(index).path());
  }

  const auto& game_root = entryAt(*mapped, GameRelativePath{""});
  REQUIRE(game_root.origin() == CompiledNamespaceEntryOrigin::MergedDirectories);
  REQUIRE_FALSE(game_root.sourceReference().has_value());

  const auto& root_file = entryAt(*mapped, GameRelativePath{"loader.so"});
  REQUIRE(root_file.origin() == CompiledNamespaceEntryOrigin::PriorityWinner);
  REQUIRE(root_file.sourceReference() ==
          std::optional<NamespaceSourceReference>{
              NamespaceSourceReference{PackageId{"high"}, PackageRelativePath{"Root/loader.so"},
                                       PackageLocation{.object_store_ref = "object:high"}}});

  const auto& content_file = entryAt(*mapped, GameRelativePath{"Data/textures/shared.dds"});
  REQUIRE(content_file.origin() == CompiledNamespaceEntryOrigin::PriorityWinner);
  REQUIRE(content_file.sourceReference().transform([](const NamespaceSourceReference& source) {
    return source.packageId();
  }) == std::optional<PackageId>{PackageId{"high"}});

  const auto& link = entryAt(*mapped, GameRelativePath{"Data/links/current"});
  REQUIRE(link.kind() == PackageEntryKind::SymbolicLink);
  REQUIRE(link.sourceReference() ==
          std::optional<NamespaceSourceReference>{NamespaceSourceReference{
              PackageId{"low"}, PackageRelativePath{"Data/links/current"},
              PackageLocation{.staging_path = std::filesystem::path{"/fixtures/staging/low"}}}});

  const auto& arbitrary = entryAt(*mapped, GameRelativePath{arbitrary_path.nativeBytes()});
  REQUIRE(arbitrary.sourceReference().transform([](const NamespaceSourceReference& source) {
    return source.packageRelativePath();
  }) == std::optional<PackageRelativePath>{arbitrary_path});
}

TEST_CASE("Synthetic empty profile completes every planning stage", "[integration][namespace]") {
  const auto fixture = fixtures::emptyFixture();
  const auto resolved = resolveFixture(fixture);
  const auto compilation = NamespaceCompilationResult::compile(resolved.conflict_report);
  REQUIRE(compilation.has_value());
  const auto profile = ProfileIdentity::create(fixture.profile_name);
  REQUIRE(profile.has_value());
  const GameFsPlan plan{*profile, *compilation};
  const auto mapped =
      plan.namespaceResult().compiledNamespace().mapSources(resolved.package_snapshot);

  REQUIRE(resolved.package_graph.resolvedOrder().empty());
  REQUIRE(resolved.ownership_graph.nodes().empty());
  REQUIRE(mapped.has_value());
  REQUIRE(mapped->entries().empty());
  REQUIRE(plan.namespaceResult().explanations().empty());
}

TEST_CASE("Synthetic blocking fixtures fail at namespace compilation", "[integration][namespace]") {
  SECTION("non-directory ancestor obstruction") {
    const auto resolved = resolveFixture(fixtures::obstructionFixture());
    REQUIRE(resolved.conflict_report.hasBlockingConflicts());
    REQUIRE(hasDiagnostic(resolved.conflict_report, ConflictDiagnosticCode::PathObstruction));

    const auto compilation = NamespaceCompilationResult::compile(resolved.conflict_report);
    REQUIRE_FALSE(compilation.has_value());
    REQUIRE(compilation.error().code() == NamespaceCompilationErrorCode::BlockingConflicts);
  }

  SECTION("ambiguous highest-priority claims") {
    const auto resolved = resolveFixture(fixtures::ambiguityFixture());
    REQUIRE(resolved.conflict_report.hasBlockingConflicts());
    REQUIRE(hasDiagnostic(resolved.conflict_report,
                          ConflictDiagnosticCode::AmbiguousHighestPriorityClaim));

    const auto compilation = NamespaceCompilationResult::compile(resolved.conflict_report);
    REQUIRE_FALSE(compilation.has_value());
    REQUIRE(compilation.error().code() == NamespaceCompilationErrorCode::BlockingConflicts);
  }
}

TEST_CASE("Synthetic source drift fails atomically at source mapping", "[integration][namespace]") {
  const auto fixture = fixtures::ordinaryFixture();
  const auto resolved = resolveFixture(fixture);
  const auto compilation = NamespaceCompilationResult::compile(resolved.conflict_report);
  REQUIRE(compilation.has_value());

  auto incomplete_plans = fixture.package_plans;
  const auto& high = incomplete_plans.at(1);
  auto high_entries = high.entries();
  std::erase_if(high_entries, [](const PackageLayoutEntry& entry) {
    return entry.relativePath() == PackageRelativePath{"Data/textures/shared.dds"};
  });
  incomplete_plans.at(1) = PackagePlan{high.packageId(), high.packageLocation(),
                                       std::move(high_entries), high.rootMappings()};

  const auto mapped =
      compilation->compiledNamespace().mapSources(packageSnapshot(incomplete_plans));

  REQUIRE_FALSE(mapped.has_value());
  REQUIRE(mapped.error().code() == NamespaceSourceMappingErrorCode::MissingSourceEntry);
  REQUIRE(mapped.error().packageId() == std::optional<PackageId>{PackageId{"high"}});
  REQUIRE(mapped.error().entryPath() ==
          std::optional<GameRelativePath>{GameRelativePath{"Data/textures/shared.dds"}});
}

TEST_CASE("Large conflict-heavy fixture selects the declared priority winners",
          "[integration][namespace]") {
  constexpr std::size_t package_count = 24;
  constexpr std::size_t path_count = 96;
  const auto fixture = fixtures::largeConflictFixture(
      fixtures::LargeConflictFixtureSize{.package_count = package_count, .path_count = path_count});
  const auto resolved = resolveFixture(fixture);

  REQUIRE_FALSE(resolved.conflict_report.hasBlockingConflicts());
  REQUIRE(resolved.ownership_graph.exactCollisionPaths().size() == path_count);
  REQUIRE(resolved.conflict_report.diagnostics().size() == path_count);
  REQUIRE(std::ranges::all_of(
      resolved.conflict_report.diagnostics(), [](const ConflictDiagnostic& diagnostic) {
        return diagnostic.code() == ConflictDiagnosticCode::ExactPathWinnerSelected;
      }));

  const auto compilation = NamespaceCompilationResult::compile(resolved.conflict_report);
  REQUIRE(compilation.has_value());
  const auto profile = ProfileIdentity::create(fixture.profile_name);
  REQUIRE(profile.has_value());
  const GameFsPlan plan{*profile, *compilation};
  const auto mapped =
      plan.namespaceResult().compiledNamespace().mapSources(resolved.package_snapshot);

  REQUIRE(mapped.has_value());
  REQUIRE(mapped->entries().size() == path_count + 2);
  const auto non_directories =
      std::ranges::count_if(mapped->entries(), [](const CompiledNamespaceEntry& entry) {
        return entry.kind() != PackageEntryKind::Directory;
      });
  REQUIRE(std::cmp_equal(non_directories, path_count));
  for (const auto& entry : mapped->entries()) {
    if (entry.kind() == PackageEntryKind::Directory) {
      REQUIRE_FALSE(entry.sourceReference().has_value());
      continue;
    }
    REQUIRE(entry.origin() == CompiledNamespaceEntryOrigin::PriorityWinner);
    REQUIRE(entry.sourceReference().transform([](const NamespaceSourceReference& source) {
      return source.packageId();
    }) == std::optional<PackageId>{PackageId{"package-23"}});
  }
}
