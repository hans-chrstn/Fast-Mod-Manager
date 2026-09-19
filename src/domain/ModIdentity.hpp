#pragma once

#include <expected>
#include <filesystem>
#include <string>

namespace fmm::domain {

enum class ModIdentityError : std::uint8_t { EmptyName, EmptyPath };

class ModIdentity {
public:
  static auto create(std::string name, std::filesystem::path staging_path)
      -> std::expected<ModIdentity, ModIdentityError> {
    if (name.empty()) {
      return std::unexpected(ModIdentityError::EmptyName);
    }
    if (staging_path.empty()) {
      return std::unexpected(ModIdentityError::EmptyPath);
    }
    return ModIdentity(std::move(name), std::move(staging_path));
  }

  [[nodiscard]] auto name() const -> const std::string& { return m_name; }
  [[nodiscard]] auto staging_path() const -> const std::filesystem::path& { return m_staging_path; }

  auto operator==(const ModIdentity&) const -> bool = default;

private:
  explicit ModIdentity(std::string name, std::filesystem::path staging_path)
      : m_name(std::move(name)), m_staging_path(std::move(staging_path)) {}

  std::string m_name;
  std::filesystem::path m_staging_path;
};

} // namespace fmm::domain
