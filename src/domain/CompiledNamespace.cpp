#include "domain/CompiledNamespace.hpp"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <string_view>

namespace fmm::domain {
namespace {

auto bytesLess(std::string_view left, std::string_view right) -> bool {
  return std::ranges::lexicographical_compare(
      left, right, [](char left_byte, char right_byte) -> bool {
        return static_cast<unsigned char>(left_byte) < static_cast<unsigned char>(right_byte);
      });
}

struct NativeByteLess final {
  auto operator()(const std::string& left, const std::string& right) const -> bool {
    return bytesLess(left, right);
  }
};

struct CompiledEntryData final {
  GameRelativePath path;
  PackageEntryKind kind{PackageEntryKind::Directory};
  CompiledNamespaceEntryOrigin origin{CompiledNamespaceEntryOrigin::SyntheticDirectory};
  std::optional<PackagePathClaim> selected_claim;
  NamespaceEntryExplanationKind explanation_kind{
      NamespaceEntryExplanationKind::SyntheticAncestorDirectory};
  std::vector<PackagePathClaim> contenders;
  std::optional<PackagePathClaim> explanation_selected_claim;
};

struct PackageSourceData final {
  const PackageLocation* location{};
  std::map<std::string, std::optional<PackageEntryKind>, NativeByteLess> entry_kinds;
};

using PackageSourceIndex = std::map<PackageId, PackageSourceData>;

auto sourceMappingError(NamespaceSourceMappingErrorCode code,
                        std::optional<PackageId> package_id = std::nullopt,
                        std::optional<PackageId> related_package_id = std::nullopt,
                        std::optional<GameRelativePath> entry_path = std::nullopt,
                        std::optional<PackageRelativePath> package_relative_path = std::nullopt)
    -> std::unexpected<NamespaceSourceMappingError> {
  return std::unexpected(
      NamespaceSourceMappingError{code, std::move(package_id), std::move(related_package_id),
                                  std::move(entry_path), std::move(package_relative_path)});
}

auto indexPackageSources(const std::vector<NamespacePackagePlanRecord>& package_snapshot)
    -> std::expected<PackageSourceIndex, NamespaceSourceMappingError> {
  PackageSourceIndex packages;
  for (const auto& record : package_snapshot) {
    const auto& lookup_id = record.lookupPackageId();
    const auto& plan = record.packagePlan();
    if (lookup_id != plan.packageId()) {
      return sourceMappingError(NamespaceSourceMappingErrorCode::MismatchedPackageId, lookup_id,
                                plan.packageId());
    }

    PackageSourceData source_data{.location = &plan.packageLocation(), .entry_kinds = {}};
    for (const auto& entry : plan.entries()) {
      auto [position, inserted] =
          source_data.entry_kinds.emplace(entry.relativePath().nativeBytes(), entry.kind());
      if (!inserted && position->second.has_value() && position->second.value() != entry.kind()) {
        position->second = std::nullopt;
      }
    }

    if (!packages.emplace(lookup_id, std::move(source_data)).second) {
      return sourceMappingError(NamespaceSourceMappingErrorCode::DuplicatePackage, lookup_id);
    }
  }
  return packages;
}

auto invalidDecision() -> std::unexpected<NamespaceCompilationError> {
  return std::unexpected(
      NamespaceCompilationError{NamespaceCompilationErrorCode::InvalidResolvedDecision});
}

auto compileDecision(const ExactPathDecision& decision)
    -> std::expected<CompiledEntryData, NamespaceCompilationError> {
  const auto& selected_claim = decision.selectedClaim();
  switch (decision.kind()) {
  case ExactPathDecisionKind::SingleClaim:
    if (decision.claims().size() != 1 || !selected_claim.has_value() ||
        selected_claim->gameRelativePath() != decision.path()) {
      return invalidDecision();
    }
    if (selected_claim->kind() == PackageEntryKind::Directory) {
      return CompiledEntryData{.path = decision.path(),
                               .kind = PackageEntryKind::Directory,
                               .origin = CompiledNamespaceEntryOrigin::SingleClaim,
                               .selected_claim = std::nullopt,
                               .explanation_kind = NamespaceEntryExplanationKind::UncontestedClaim,
                               .contenders = decision.claims(),
                               .explanation_selected_claim = selected_claim};
    }
    return CompiledEntryData{.path = decision.path(),
                             .kind = selected_claim->kind(),
                             .origin = CompiledNamespaceEntryOrigin::SingleClaim,
                             .selected_claim = selected_claim,
                             .explanation_kind = NamespaceEntryExplanationKind::UncontestedClaim,
                             .contenders = decision.claims(),
                             .explanation_selected_claim = selected_claim};
  case ExactPathDecisionKind::MergedDirectories:
    if (decision.claims().empty() || selected_claim.has_value() ||
        !std::ranges::all_of(decision.claims(), [](const PackagePathClaim& claim) -> bool {
          return claim.kind() == PackageEntryKind::Directory;
        })) {
      return invalidDecision();
    }
    return CompiledEntryData{.path = decision.path(),
                             .kind = PackageEntryKind::Directory,
                             .origin = CompiledNamespaceEntryOrigin::MergedDirectories,
                             .selected_claim = std::nullopt,
                             .explanation_kind = NamespaceEntryExplanationKind::MergedDirectory,
                             .contenders = decision.claims(),
                             .explanation_selected_claim = std::nullopt};
  case ExactPathDecisionKind::PriorityWinner:
    if (!selected_claim.has_value() || selected_claim->gameRelativePath() != decision.path() ||
        selected_claim->kind() == PackageEntryKind::Directory) {
      return invalidDecision();
    }
    return CompiledEntryData{.path = decision.path(),
                             .kind = selected_claim->kind(),
                             .origin = CompiledNamespaceEntryOrigin::PriorityWinner,
                             .selected_claim = selected_claim,
                             .explanation_kind = NamespaceEntryExplanationKind::PriorityWinner,
                             .contenders = decision.claims(),
                             .explanation_selected_claim = selected_claim};
  case ExactPathDecisionKind::Unresolved:
    return invalidDecision();
  }

  return invalidDecision();
}

auto syntheticAncestors(const std::vector<ExactPathDecision>& decisions)
    -> std::vector<GameRelativePath> {
  std::set<std::string, NativeByteLess> occupied_paths;
  for (const auto& decision : decisions) {
    occupied_paths.insert(decision.path().nativeBytes());
  }

  std::vector<GameRelativePath> ancestors;
  for (const auto& decision : decisions) {
    const auto& path = decision.path().nativeBytes();
    for (auto separator = path.find('/'); separator != std::string::npos;
         separator = path.find('/', separator + 1)) {
      if (separator == 0) {
        continue;
      }

      auto ancestor = path.substr(0, separator);
      if (occupied_paths.insert(ancestor).second) {
        ancestors.emplace_back(std::move(ancestor));
      }
    }
  }
  return ancestors;
}

} // namespace

auto CompiledNamespace::compile(const ConflictResolutionReport& report)
    -> std::expected<CompiledNamespace, NamespaceCompilationError> {
  auto result = NamespaceCompilationResult::compile(report);
  if (!result.has_value()) {
    return std::unexpected(result.error());
  }
  return std::move(result->m_compiled_namespace);
}

auto NamespaceCompilationResult::compile(const ConflictResolutionReport& report)
    -> std::expected<NamespaceCompilationResult, NamespaceCompilationError> {
  if (report.hasBlockingConflicts()) {
    return std::unexpected(
        NamespaceCompilationError{NamespaceCompilationErrorCode::BlockingConflicts});
  }

  std::vector<CompiledEntryData> compilation_data;
  compilation_data.reserve(report.decisions().size());

  for (const auto& decision : report.decisions()) {
    auto entry = compileDecision(decision);
    if (!entry.has_value()) {
      return std::unexpected(entry.error());
    }
    compilation_data.push_back(std::move(entry).value());
  }

  auto ancestors = syntheticAncestors(report.decisions());
  compilation_data.reserve(compilation_data.size() + ancestors.size());
  std::ranges::transform(ancestors, std::back_inserter(compilation_data),
                         [](GameRelativePath& ancestor) -> CompiledEntryData {
                           return CompiledEntryData{
                               .path = std::move(ancestor),
                               .kind = PackageEntryKind::Directory,
                               .origin = CompiledNamespaceEntryOrigin::SyntheticDirectory,
                               .selected_claim = std::nullopt,
                               .explanation_kind =
                                   NamespaceEntryExplanationKind::SyntheticAncestorDirectory,
                               .contenders = {},
                               .explanation_selected_claim = std::nullopt};
                         });

  std::ranges::sort(compilation_data,
                    [](const CompiledEntryData& left, const CompiledEntryData& right) -> bool {
                      return bytesLess(left.path.nativeBytes(), right.path.nativeBytes());
                    });

  std::vector<CompiledNamespaceEntry> entries;
  std::vector<NamespaceEntryExplanation> explanations;
  entries.reserve(compilation_data.size());
  explanations.reserve(compilation_data.size());
  for (auto& data : compilation_data) {
    auto explanation_path = data.path;
    entries.push_back(CompiledNamespaceEntry{std::move(data.path), data.kind, data.origin,
                                             std::move(data.selected_claim)});
    explanations.push_back(NamespaceEntryExplanation{
        std::move(explanation_path), data.explanation_kind, std::move(data.contenders),
        std::move(data.explanation_selected_claim)});
  }

  return NamespaceCompilationResult{CompiledNamespace{std::move(entries)}, report,
                                    std::move(explanations)};
}

auto CompiledNamespace::mapSources(const std::vector<NamespacePackagePlanRecord>& package_snapshot)
    const -> std::expected<SourceMappedNamespace, NamespaceSourceMappingError> {
  auto packages = indexPackageSources(package_snapshot);
  if (!packages.has_value()) {
    return std::unexpected(packages.error());
  }

  auto mapped_entries = m_entries;
  for (auto& entry : mapped_entries) {
    if (entry.kind() == PackageEntryKind::Directory) {
      if (entry.selectedClaim().has_value() || entry.sourceReference().has_value()) {
        return sourceMappingError(NamespaceSourceMappingErrorCode::InvalidEntryKind, std::nullopt,
                                  std::nullopt, entry.path());
      }
      continue;
    }

    if (!entry.selectedClaim().has_value() || entry.selectedClaim()->kind() != entry.kind() ||
        entry.selectedClaim()->kind() == PackageEntryKind::Directory) {
      return sourceMappingError(NamespaceSourceMappingErrorCode::InvalidEntryKind, std::nullopt,
                                std::nullopt, entry.path());
    }

    const auto& selected = entry.selectedClaim().value();
    const auto package = packages->find(selected.packageId());
    if (package == packages->end()) {
      return sourceMappingError(NamespaceSourceMappingErrorCode::MissingPackage,
                                selected.packageId(), std::nullopt, entry.path(),
                                selected.packageRelativePath());
    }

    const auto source_entry =
        package->second.entry_kinds.find(selected.packageRelativePath().nativeBytes());
    if (source_entry == package->second.entry_kinds.end()) {
      return sourceMappingError(NamespaceSourceMappingErrorCode::MissingSourceEntry,
                                selected.packageId(), std::nullopt, entry.path(),
                                selected.packageRelativePath());
    }
    if (!source_entry->second.has_value() || source_entry->second.value() != entry.kind() ||
        source_entry->second.value() == PackageEntryKind::Directory) {
      return sourceMappingError(NamespaceSourceMappingErrorCode::InvalidEntryKind,
                                selected.packageId(), std::nullopt, entry.path(),
                                selected.packageRelativePath());
    }

    entry.m_source_reference = NamespaceSourceReference{
        selected.packageId(), selected.packageRelativePath(), *package->second.location};
  }

  return SourceMappedNamespace{std::move(mapped_entries)};
}

} // namespace fmm::domain
