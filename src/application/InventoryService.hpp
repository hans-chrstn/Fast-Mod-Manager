#pragma once

#include "domain/ModIdentity.hpp"

#include <vector>

namespace application {

class InventoryService {
public:
  InventoryService() = default;
  virtual ~InventoryService() = default;

  [[nodiscard]] virtual auto getInventory() const -> std::vector<domain::ModIdentity> = 0;
};

} // namespace application
