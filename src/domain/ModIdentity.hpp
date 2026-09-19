#pragma once

#include <string>

namespace domain {

class ModIdentity {
public:
  explicit ModIdentity(std::string name) : m_name(std::move(name)) {}

  [[nodiscard]] auto name() const -> const std::string& { return m_name; }

  auto operator==(const ModIdentity&) const -> bool = default;

private:
  std::string m_name;
};

} // namespace domain
