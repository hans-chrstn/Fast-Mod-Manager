#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace fmm::domain {

enum class ProfileIdentityError : std::uint8_t {
  EmptyName,
  InvalidCharacters,
  TrailingOrLeadingWhitespace,
  NameTooLong,
  ReservedName
};

class ProfileIdentity {
public:
  static auto create(std::string_view name,
                     std::optional<std::string_view> parent_id = std::nullopt)
      -> std::expected<ProfileIdentity, ProfileIdentityError>;

  [[nodiscard]] auto name() const noexcept -> std::string_view;
  [[nodiscard]] auto parent_id() const noexcept -> std::optional<std::string_view>;

  auto operator==(const ProfileIdentity& other) const noexcept -> bool;

private:
  explicit ProfileIdentity(std::string_view name, std::optional<std::string_view> parent_id);

  std::string m_name;
  std::optional<std::string> m_parent_id;
};

} // namespace fmm::domain
