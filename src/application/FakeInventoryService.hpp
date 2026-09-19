#pragma once

#include "application/InventoryService.hpp"

namespace application {

class FakeInventoryService : public InventoryService {
public:
  [[nodiscard]] auto getInventory() const -> std::vector<domain::ModIdentity> override {
    return {domain::ModIdentity{"Unofficial Patch"}, domain::ModIdentity{"High Res Textures"},
            domain::ModIdentity{"UI Overhaul"}, domain::ModIdentity{"Alternate Start"}};
  }
};

} // namespace application
