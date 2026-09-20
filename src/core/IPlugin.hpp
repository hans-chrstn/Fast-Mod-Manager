#pragma once

#include "core/GameFsPlan.hpp"
#include "core/LaunchPlan.hpp"
#include "core/PackagePlan.hpp"

#include <QtPlugin>
#include <string>

namespace core {

class IPlugin {
public:
  virtual ~IPlugin() = default;

  [[nodiscard]] virtual auto name() const -> std::string = 0;
  virtual void initialize() = 0;

  virtual void contributePackagePlan(PackagePlan& plan) = 0;
  virtual void contributeGameFsPlan(GameFsPlan& plan) = 0;
  virtual void contributeLaunchPlan(LaunchPlan& plan) = 0;
};

} // namespace core

Q_DECLARE_INTERFACE(core::IPlugin, "com.fastmodmanager.IPlugin/1.0")
