#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fmm::domain {

struct VirtualNode {
  std::string name;
  bool is_directory{false};
  std::uint64_t size{0};
  std::vector<VirtualNode> children;
};

class VirtualTree {
public:
  [[nodiscard]] auto root() const -> const VirtualNode& { return m_root; }
  auto root() -> VirtualNode& { return m_root; }

private:
  VirtualNode m_root{.name = "", .is_directory = true, .size = 0, .children = {}};
};

} // namespace fmm::domain
