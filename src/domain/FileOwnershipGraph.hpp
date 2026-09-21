#pragma once

#include "domain/PackageDependencyGraph.hpp"
#include "domain/PackagePlan.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <utility>
#include <vector>

namespace fmm::domain {

class PackagePathClaim final {
public:
  PackagePathClaim(PackageId package_id, PackageRelativePath package_relative_path,
                   GameRelativePath game_relative_path, PackageEntryKind kind)
      : m_package_id(std::move(package_id)),
        m_package_relative_path(std::move(package_relative_path)),
        m_game_relative_path(std::move(game_relative_path)), m_kind(kind) {}

  [[nodiscard]] auto packageId() const noexcept -> const PackageId& { return m_package_id; }

  [[nodiscard]] auto packageRelativePath() const noexcept -> const PackageRelativePath& {
    return m_package_relative_path;
  }

  [[nodiscard]] auto gameRelativePath() const noexcept -> const GameRelativePath& {
    return m_game_relative_path;
  }

  [[nodiscard]] auto kind() const noexcept -> PackageEntryKind { return m_kind; }

  auto operator==(const PackagePathClaim&) const -> bool = default;

private:
  PackageId m_package_id;
  PackageRelativePath m_package_relative_path;
  GameRelativePath m_game_relative_path;
  PackageEntryKind m_kind;
};

class FileOwnershipNode final {
public:
  FileOwnershipNode(GameRelativePath path, std::vector<PackagePathClaim> claims)
      : m_path(std::move(path)), m_claims(std::move(claims)) {}

  [[nodiscard]] auto path() const noexcept -> const GameRelativePath& { return m_path; }

  [[nodiscard]] auto claims() const noexcept -> const std::vector<PackagePathClaim>& {
    return m_claims;
  }

  auto operator==(const FileOwnershipNode&) const -> bool = default;

private:
  GameRelativePath m_path;
  std::vector<PackagePathClaim> m_claims;
};

class PathObstruction final {
public:
  PathObstruction(GameRelativePath ancestor_path, GameRelativePath descendant_path)
      : m_ancestor_path(std::move(ancestor_path)), m_descendant_path(std::move(descendant_path)) {}

  [[nodiscard]] auto ancestorPath() const noexcept -> const GameRelativePath& {
    return m_ancestor_path;
  }

  [[nodiscard]] auto descendantPath() const noexcept -> const GameRelativePath& {
    return m_descendant_path;
  }

  auto operator==(const PathObstruction&) const -> bool = default;

private:
  GameRelativePath m_ancestor_path;
  GameRelativePath m_descendant_path;
};

enum class FileOwnershipGraphErrorCode : std::uint8_t { ClaimPackageNotEnabled };

class FileOwnershipGraphError final {
public:
  FileOwnershipGraphError(FileOwnershipGraphErrorCode code, PackageId package_id,
                          std::size_t claim_index)
      : m_code(code), m_package_id(std::move(package_id)), m_claim_index(claim_index) {}

  [[nodiscard]] auto code() const noexcept -> FileOwnershipGraphErrorCode { return m_code; }

  [[nodiscard]] auto packageId() const noexcept -> const PackageId& { return m_package_id; }

  [[nodiscard]] auto claimIndex() const noexcept -> std::size_t { return m_claim_index; }

  auto operator==(const FileOwnershipGraphError&) const -> bool = default;

private:
  FileOwnershipGraphErrorCode m_code;
  PackageId m_package_id;
  std::size_t m_claim_index;
};

class FileOwnershipGraph final {
public:
  [[nodiscard]] static auto create(const PackageDependencyGraph& package_graph,
                                   std::vector<PackagePathClaim> claims)
      -> std::expected<FileOwnershipGraph, FileOwnershipGraphError>;

  [[nodiscard]] auto packageOrder() const noexcept -> const std::vector<PackageId>& {
    return m_package_order;
  }

  [[nodiscard]] auto claims() const noexcept -> const std::vector<PackagePathClaim>& {
    return m_claims;
  }

  [[nodiscard]] auto nodes() const noexcept -> const std::vector<FileOwnershipNode>& {
    return m_nodes;
  }

  [[nodiscard]] auto exactCollisionPaths() const noexcept -> const std::vector<GameRelativePath>& {
    return m_exact_collision_paths;
  }

  [[nodiscard]] auto obstructions() const noexcept -> const std::vector<PathObstruction>& {
    return m_obstructions;
  }

  auto operator==(const FileOwnershipGraph&) const -> bool = default;

private:
  FileOwnershipGraph(std::vector<PackageId> package_order, std::vector<PackagePathClaim> claims,
                     std::vector<FileOwnershipNode> nodes,
                     std::vector<GameRelativePath> exact_collision_paths,
                     std::vector<PathObstruction> obstructions)
      : m_package_order(std::move(package_order)), m_claims(std::move(claims)),
        m_nodes(std::move(nodes)), m_exact_collision_paths(std::move(exact_collision_paths)),
        m_obstructions(std::move(obstructions)) {}

  std::vector<PackageId> m_package_order;
  std::vector<PackagePathClaim> m_claims;
  std::vector<FileOwnershipNode> m_nodes;
  std::vector<GameRelativePath> m_exact_collision_paths;
  std::vector<PathObstruction> m_obstructions;
};

} // namespace fmm::domain
