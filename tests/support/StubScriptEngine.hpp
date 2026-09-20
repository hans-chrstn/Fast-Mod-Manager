#pragma once

#include "core/IScriptEngine.hpp"

namespace fmm::test_support {

class StubScriptEngine : public core::IScriptEngine {
public:
  [[nodiscard]] auto evaluateGamePlugin(const std::string& script_content) const
      -> std::expected<domain::GameDefinition, core::GamePluginError> override;
};

} // namespace fmm::test_support
