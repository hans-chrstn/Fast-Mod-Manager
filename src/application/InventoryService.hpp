#pragma once

#include "domain/ModIdentity.hpp"

#include <expected>
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
      -> std::expected<std::vector<fmm::domain::ModIdentity>, std::string> = 0;
};

} // namespace application
