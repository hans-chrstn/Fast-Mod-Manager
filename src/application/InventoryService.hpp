#pragma once

#include "domain/ModIdentity.hpp"

#include <functional>
#include <stop_token>
#include <string>
#include <vector>

namespace application {

class InventoryService {
public:
  InventoryService() = default;
  virtual ~InventoryService() = default;

  [[nodiscard]] virtual auto
  getInventory(const std::stop_token& stoken = {},
               const std::function<void(int, const std::string&)>& progress_callback = {}) const
      -> std::vector<fmm::domain::ModIdentity> = 0;
};

} // namespace application
