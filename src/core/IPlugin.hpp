#pragma once

#include <QString>
#include <QtPlugin>

namespace core {

class ServiceRegistry;

class IPlugin {
public:
  virtual ~IPlugin() = default;

  [[nodiscard]] virtual auto name() const -> QString = 0;
  virtual void initialize(ServiceRegistry& registry) = 0;
};

} // namespace core

Q_DECLARE_INTERFACE(core::IPlugin, "com.fastmodmanager.IPlugin/1.0")
