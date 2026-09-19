#pragma once

#include "core/IScriptEngine.hpp"

namespace infrastructure {

class StubScriptEngine : public core::IScriptEngine {
public:
  [[nodiscard]] auto evaluate(const std::string& script) const
      -> std::expected<std::string, std::string> override;
};

} // namespace infrastructure
