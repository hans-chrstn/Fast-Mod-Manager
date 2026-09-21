#pragma once

#include <compare>
#include <string>
#include <utility>

namespace fmm::domain {

class PluginId {
public:
  explicit PluginId(std::string identifier) : m_identifier(std::move(identifier)) {}

  [[nodiscard]] auto value() const noexcept -> const std::string& { return m_identifier; }

  auto operator==(const PluginId&) const -> bool = default;
  auto operator<=>(const PluginId&) const = default;

private:
  std::string m_identifier;
};

} // namespace fmm::domain
