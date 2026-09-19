#include "domain/ProfileIdentity.hpp"

#include <algorithm>

namespace fmm::domain {

constexpr size_t max_profile_name_length = 255;

auto ProfileIdentity::create(std::string_view name, std::optional<std::string_view> parent_id)
    -> std::expected<ProfileIdentity, ProfileIdentityError> {
  if (name.empty()) {
    return std::unexpected(ProfileIdentityError::EmptyName);
  }

  if (name.front() == ' ' || name.back() == ' ') {
    return std::unexpected(ProfileIdentityError::TrailingOrLeadingWhitespace);
  }

  if (name.length() > max_profile_name_length) {
    return std::unexpected(ProfileIdentityError::NameTooLong);
  }

  if (name == "." || name == "..") {
    return std::unexpected(ProfileIdentityError::ReservedName);
  }

  auto has_invalid_chars = [](char character) -> bool {
    return character == '/' || character == '\\' || character == '\0' || character == ':' ||
           character == '*' || character == '?' || character == '"' || character == '<' ||
           character == '>' || character == '|';
  };

  if (std::ranges::any_of(name, has_invalid_chars)) {
    return std::unexpected(ProfileIdentityError::InvalidCharacters);
  }

  return ProfileIdentity(name, parent_id);
}

ProfileIdentity::ProfileIdentity(std::string_view name, std::optional<std::string_view> parent_id)
    : m_name(name) {
  if (parent_id) {
    m_parent_id = std::string(*parent_id);
  }
}

auto ProfileIdentity::name() const noexcept -> std::string_view { return m_name; }

auto ProfileIdentity::parent_id() const noexcept -> std::optional<std::string_view> {
  if (m_parent_id) {
    return std::string_view(*m_parent_id);
  }
  return std::nullopt;
}

auto ProfileIdentity::operator==(const ProfileIdentity& other) const noexcept -> bool {
  return m_name == other.m_name && m_parent_id == other.m_parent_id;
}

} // namespace fmm::domain
