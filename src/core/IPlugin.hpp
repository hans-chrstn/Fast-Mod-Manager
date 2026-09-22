#pragma once

#include "core/LaunchPlan.hpp"
#include "core/LegacyGameFsAbiPlan.hpp"
#include "domain/PackagePlan.hpp"

#include <QtPlugin>
#include <string>

namespace fmm::core {

class IPlugin {
public:
  virtual ~IPlugin() = default;

  [[nodiscard]] virtual auto name() const -> std::string = 0;
  virtual void initialize() = 0;

  virtual void contributePackagePlan(domain::PackagePlan& plan) = 0;
  virtual void contributeGameFsPlan(LegacyGameFsAbiPlan& plan) = 0;
  virtual void contributeLaunchPlan(LaunchPlan& plan) = 0;
};

} // namespace fmm::core

Q_DECLARE_INTERFACE(fmm::core::IPlugin, "com.fastmodmanager.IPlugin/1.0")
