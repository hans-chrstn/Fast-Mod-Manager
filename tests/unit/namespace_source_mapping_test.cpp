#include "domain/CompiledNamespace.hpp"

#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace fmm::domain;

static_assert(std::is_same_v<decltype(std::declval<const CompiledNamespace&>().mapSources(
                                 std::declval<const std::vector<NamespacePackagePlanRecord>&>())),
                             std::expected<SourceMappedNamespace, NamespaceSourceMappingError>>);
static_assert(std::is_same_v<decltype(std::declval<const SourceMappedNamespace&>().entries()),
                             const std::vector<CompiledNamespaceEntry>&>);
static_assert(
    std::is_same_v<decltype(std::declval<const CompiledNamespaceEntry&>().sourceReference()),
                   const std::optional<NamespaceSourceReference>&>);
static_assert(!std::is_default_constructible_v<NamespaceSourceReference>);
static_assert(!std::is_default_constructible_v<NamespacePackagePlanRecord>);
static_assert(!std::is_default_constructible_v<SourceMappedNamespace>);

namespace {

auto claim(std::string package, std::string source, std::string destination,
           PackageEntryKind kind = PackageEntryKind::RegularFile) -> PackagePathClaim {
  return PackagePathClaim{PackageId{std::move(package)}, PackageRelativePath{std::move(source)},
                          GameRelativePath{std::move(destination)}, kind};
}

auto compile(std::vector<PackageId> packages, std::vector<PackagePathClaim> claims,
             std::vector<PackageId> priority) -> CompiledNamespace {
  const auto package_graph = PackageDependencyGraph::create(std::move(packages), {}, {});
  REQUIRE(package_graph.has_value());
  const auto ownership = FileOwnershipGraph::create(*package_graph, std::move(claims));
  REQUIRE(ownership.has_value());
  const auto resolved = ConflictResolutionReport::create(*ownership, std::move(priority));
  REQUIRE(resolved.has_value());
  const auto compiled = CompiledNamespace::compile(*resolved);
  REQUIRE(compiled.has_value());
  return *compiled;
}

auto planRecord(std::string lookup_id, std::string plan_id, PackageLocation location,
                std::vector<PackageLayoutEntry> entries) -> NamespacePackagePlanRecord {
  return NamespacePackagePlanRecord{
      PackageId{std::move(lookup_id)},
      PackagePlan{PackageId{std::move(plan_id)}, std::move(location), std::move(entries), {}}};
}

auto sourceEntry(std::string path, PackageEntryKind kind = PackageEntryKind::RegularFile)
    -> PackageLayoutEntry {
  return PackageLayoutEntry{PackageRelativePath{std::move(path)}, kind};
}

auto requireError(std::expected<SourceMappedNamespace, NamespaceSourceMappingError> result,
                  NamespaceSourceMappingErrorCode code) -> NamespaceSourceMappingError {
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().code() == code);
  return result.error();
}

} // namespace

TEST_CASE("Namespace source mapping owns direct staging and object-store references",
          "[domain][namespace][source]") {
  std::string regular_source{"layout/"};
  regular_source.push_back(static_cast<char>(0xFE));
  std::string regular_destination{"Data/"};
  regular_destination.push_back(static_cast<char>(0xFF));

  const auto expected_source = regular_source;
  const auto expected_destination = regular_destination;
  const auto compiled =
      compile({PackageId{"staging"}, PackageId{"object"}},
              {claim("staging", regular_source, regular_destination),
               claim("object", "links/current", "Links/current", PackageEntryKind::SymbolicLink)},
              {PackageId{"staging"}, PackageId{"object"}});

  std::vector<NamespacePackagePlanRecord> snapshot;
  snapshot.push_back(
      planRecord("staging", "staging",
                 PackageLocation{.staging_path = std::filesystem::path{"/managed/staging"}},
                 {sourceEntry(regular_source)}));
  snapshot.push_back(planRecord("object", "object",
                                PackageLocation{.object_store_ref = "sha256:object"},
                                {sourceEntry("links/current", PackageEntryKind::SymbolicLink)}));

  const auto first = compiled.mapSources(snapshot);
  const auto second = compiled.mapSources(snapshot);
  regular_source.clear();
  regular_destination.clear();
  snapshot.clear();

  REQUIRE(first.has_value());
  REQUIRE(second.has_value());
  REQUIRE(first == second);
  REQUIRE(first->entries().size() == 4);

  const auto& data_directory = first->entries().at(0);
  REQUIRE(data_directory.path() == GameRelativePath{"Data"});
  REQUIRE(data_directory.origin() == CompiledNamespaceEntryOrigin::SyntheticDirectory);
  REQUIRE_FALSE(data_directory.sourceReference().has_value());

  const auto& regular_file = first->entries().at(1);
  REQUIRE(regular_file.path() == GameRelativePath{expected_destination});
  REQUIRE(regular_file.sourceReference().has_value());
  REQUIRE(regular_file.sourceReference()->packageId() == PackageId{"staging"});
  REQUIRE(regular_file.sourceReference()->packageRelativePath() ==
          PackageRelativePath{expected_source});
  REQUIRE(regular_file.sourceReference()->packageLocation() ==
          PackageLocation{.staging_path = std::filesystem::path{"/managed/staging"}});

  const auto& link = first->entries().at(3);
  REQUIRE(link.kind() == PackageEntryKind::SymbolicLink);
  REQUIRE(link.sourceReference().has_value());
  REQUIRE(link.sourceReference()->packageId() == PackageId{"object"});
  REQUIRE(link.sourceReference()->packageRelativePath() == PackageRelativePath{"links/current"});
  REQUIRE(link.sourceReference()->packageLocation() ==
          PackageLocation{.object_store_ref = "sha256:object"});
}

TEST_CASE("Namespace source mapping preserves explicit directories, winners, and order",
          "[domain][namespace][source]") {
  const auto winner = claim("high", "winner/file", "Shared/file");
  const auto compiled = compile({PackageId{"low"}, PackageId{"high"}},
                                {claim("low", "directory", "Explicit", PackageEntryKind::Directory),
                                 claim("low", "loser/file", "Shared/file"), winner},
                                {PackageId{"low"}, PackageId{"high"}});
  const std::vector<NamespacePackagePlanRecord> snapshot{
      planRecord(
          "low", "low", PackageLocation{.staging_path = "/managed/low"},
          {sourceEntry("directory", PackageEntryKind::Directory), sourceEntry("loser/file")}),
      planRecord("high", "high", PackageLocation{.object_store_ref = "object:high"},
                 {sourceEntry("winner/file")}),
  };

  const auto mapped = compiled.mapSources(snapshot);

  REQUIRE(mapped.has_value());
  REQUIRE(mapped->entries().size() == compiled.entries().size());
  for (std::size_t index = 0; index < mapped->entries().size(); ++index) {
    const auto& mapped_entry = mapped->entries().at(index);
    REQUIRE(mapped_entry.path() == compiled.entries().at(index).path());
    REQUIRE(mapped_entry.kind() == compiled.entries().at(index).kind());
    REQUIRE(mapped_entry.origin() == compiled.entries().at(index).origin());
    REQUIRE(mapped_entry.selectedClaim() == compiled.entries().at(index).selectedClaim());
    if (mapped_entry.kind() == PackageEntryKind::Directory) {
      REQUIRE_FALSE(mapped_entry.sourceReference().has_value());
    } else {
      REQUIRE(mapped_entry.sourceReference().has_value());
    }
  }

  REQUIRE(mapped->entries().at(0).path() == GameRelativePath{"Explicit"});
  REQUIRE(mapped->entries().at(0).origin() == CompiledNamespaceEntryOrigin::SingleClaim);
  REQUIRE_FALSE(mapped->entries().at(0).sourceReference().has_value());
  REQUIRE(mapped->entries().at(1).path() == GameRelativePath{"Shared"});
  REQUIRE(mapped->entries().at(1).origin() == CompiledNamespaceEntryOrigin::SyntheticDirectory);
  REQUIRE_FALSE(mapped->entries().at(1).sourceReference().has_value());
  REQUIRE(mapped->entries().at(2).origin() == CompiledNamespaceEntryOrigin::PriorityWinner);
  REQUIRE(mapped->entries().at(2).sourceReference()->packageId() == PackageId{"high"});
}

TEST_CASE("Namespace source mapping rejects invalid package snapshots atomically",
          "[domain][namespace][source]") {
  const auto compiled =
      compile({PackageId{"package"}}, {claim("package", "source/file", "Data/file")},
              {PackageId{"package"}});

  SECTION("missing package") {
    const auto error =
        requireError(compiled.mapSources({}), NamespaceSourceMappingErrorCode::MissingPackage);
    REQUIRE(error.packageId() == std::optional<PackageId>{PackageId{"package"}});
    REQUIRE(error.entryPath() == std::optional<GameRelativePath>{GameRelativePath{"Data/file"}});
    REQUIRE(error.packageRelativePath() ==
            std::optional<PackageRelativePath>{PackageRelativePath{"source/file"}});
  }

  SECTION("duplicate package") {
    const auto record =
        planRecord("package", "package", PackageLocation{}, {sourceEntry("source/file")});
    const auto error = requireError(compiled.mapSources({record, record}),
                                    NamespaceSourceMappingErrorCode::DuplicatePackage);
    REQUIRE(error.packageId() == std::optional<PackageId>{PackageId{"package"}});
  }

  SECTION("snapshot key and plan ID mismatch") {
    const auto error =
        requireError(compiled.mapSources({planRecord("package", "different", PackageLocation{},
                                                     {sourceEntry("source/file")})}),
                     NamespaceSourceMappingErrorCode::MismatchedPackageId);
    REQUIRE(error.packageId() == std::optional<PackageId>{PackageId{"package"}});
    REQUIRE(error.relatedPackageId() == std::optional<PackageId>{PackageId{"different"}});
  }

  SECTION("missing source entry") {
    const auto error =
        requireError(compiled.mapSources({planRecord("package", "package", PackageLocation{},
                                                     {sourceEntry("other")})}),
                     NamespaceSourceMappingErrorCode::MissingSourceEntry);
    REQUIRE(error.packageRelativePath() ==
            std::optional<PackageRelativePath>{PackageRelativePath{"source/file"}});
  }

  SECTION("source entry kind mismatch") {
    const auto error =
        requireError(compiled.mapSources(
                         {planRecord("package", "package", PackageLocation{},
                                     {sourceEntry("source/file", PackageEntryKind::Directory)})}),
                     NamespaceSourceMappingErrorCode::InvalidEntryKind);
    REQUIRE(error.entryPath() == std::optional<GameRelativePath>{GameRelativePath{"Data/file"}});
  }

  SECTION("ambiguous source entry kinds") {
    const auto error = requireError(
        compiled.mapSources(
            {planRecord("package", "package", PackageLocation{},
                        {sourceEntry("source/file"),
                         sourceEntry("source/file", PackageEntryKind::SymbolicLink)})}),
        NamespaceSourceMappingErrorCode::InvalidEntryKind);
    REQUIRE(error.packageId() == std::optional<PackageId>{PackageId{"package"}});
  }
}

TEST_CASE("Empty namespaces map without package records", "[domain][namespace][source]") {
  const auto compiled = compile({}, {}, {});
  const auto mapped = compiled.mapSources({});

  REQUIRE(mapped.has_value());
  REQUIRE(mapped->entries().empty());
}
