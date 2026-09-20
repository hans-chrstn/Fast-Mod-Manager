#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

namespace core {

class ServiceRegistry {
public:
  ServiceRegistry() = default;
  ~ServiceRegistry() = default;

  template <typename Interface> void registerService(const std::shared_ptr<Interface>& service) {
    if (!service) {
      throw std::invalid_argument("Cannot register a null service");
    }
    auto type_idx = std::type_index(typeid(Interface));
    if (m_services.contains(type_idx)) {
      throw std::logic_error("Service already registered");
    }
    m_services[type_idx] = std::static_pointer_cast<void>(service);
  }

  template <typename Interface> [[nodiscard]] auto resolve() const -> std::shared_ptr<Interface> {
    auto iterator = m_services.find(std::type_index(typeid(Interface)));
    if (iterator == m_services.end()) {
      throw std::runtime_error("Service not found in registry");
    }
    return std::static_pointer_cast<Interface>(iterator->second);
  }

  template <typename Interface> [[nodiscard]] auto hasService() const -> bool {
    return m_services.contains(std::type_index(typeid(Interface)));
  }

private:
  std::unordered_map<std::type_index, std::shared_ptr<void>> m_services;
};

} // namespace core
