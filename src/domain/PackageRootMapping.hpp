#pragma once

#include <optional>
#include <string>
#include <utility>

namespace fmm::domain {

class PackageRelativePath final {
public:
  explicit PackageRelativePath(std::string native_bytes)
      : m_native_bytes(std::move(native_bytes)) {}

  [[nodiscard]] auto nativeBytes() const noexcept -> const std::string& { return m_native_bytes; }

  auto operator==(const PackageRelativePath&) const -> bool = default;

private:
  std::string m_native_bytes;
};

class GameRelativePath final {
public:
  explicit GameRelativePath(std::string native_bytes) : m_native_bytes(std::move(native_bytes)) {}

  [[nodiscard]] auto nativeBytes() const noexcept -> const std::string& { return m_native_bytes; }

  auto operator==(const GameRelativePath&) const -> bool = default;

private:
  std::string m_native_bytes;
};

class PackageRootMapping final {
public:
  PackageRootMapping(PackageRelativePath package_root, GameRelativePath game_root)
      : m_package_root(std::move(package_root)), m_game_root(std::move(game_root)) {}

  [[nodiscard]] auto packageRoot() const noexcept -> const PackageRelativePath& {
    return m_package_root;
  }

  [[nodiscard]] auto gameRoot() const noexcept -> const GameRelativePath& { return m_game_root; }

  [[nodiscard]] auto map(const PackageRelativePath& package_path) const
      -> std::optional<GameRelativePath> {
    const auto& package_root_bytes = m_package_root.nativeBytes();
    const auto& package_path_bytes = package_path.nativeBytes();

    if (package_root_bytes.empty()) {
      return std::nullopt;
    }

    if (package_path_bytes == package_root_bytes) {
      return m_game_root;
    }

    if (package_path_bytes.size() <= package_root_bytes.size() ||
        !package_path_bytes.starts_with(package_root_bytes) ||
        package_path_bytes.at(package_root_bytes.size()) != '/') {
      return std::nullopt;
    }

    const auto suffix = package_path_bytes.substr(package_root_bytes.size() + 1);
    if (m_game_root.nativeBytes().empty()) {
      return GameRelativePath{suffix};
    }

    auto mapped_bytes = m_game_root.nativeBytes();
    mapped_bytes += '/';
    mapped_bytes += suffix;
    return GameRelativePath{std::move(mapped_bytes)};
  }

  auto operator==(const PackageRootMapping&) const -> bool = default;

private:
  PackageRelativePath m_package_root;
  GameRelativePath m_game_root;
};

} // namespace fmm::domain
