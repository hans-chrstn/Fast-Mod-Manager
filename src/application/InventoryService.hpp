#pragma once

#include "application/InventoryError.hpp"
#include "domain/InstalledPackage.hpp"

#include <expected>
#include <functional>
#include <stop_token>
#include <string>
#include <vector>

namespace fmm::application {

class InventoryService {
public:
  virtual ~InventoryService() = default;

  [[nodiscard]] virtual auto
  getInventory(const std::stop_token& stoken = {},
               const std::function<void(int, const std::string&)>& progress_callback = {}) const
      -> std::expected<std::vector<fmm::domain::InstalledPackage>, InventoryError> = 0;
};

} // namespace fmm::application
