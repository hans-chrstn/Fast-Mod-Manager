#pragma once

#include <string>
#include <string_view>

namespace fmm::domain {

class PackageId {
public:
  explicit PackageId(std::string identifier) : m_identifier(std::move(identifier)) {}

  [[nodiscard]] auto value() const -> const std::string& { return m_identifier; }

  auto operator==(const PackageId&) const -> bool = default;
  auto operator<=>(const PackageId&) const = default;

private:
  std::string m_identifier;
};

} // namespace fmm::domain
