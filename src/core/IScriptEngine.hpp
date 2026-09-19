#pragma once

#include <expected>
#include <string>

namespace fmm::core {

class IScriptEngine {
public:
  virtual ~IScriptEngine() = default;

  [[nodiscard]] virtual auto evaluate(const std::string& script) const
      -> std::expected<std::string, std::string> = 0;
};

} // namespace fmm::core
