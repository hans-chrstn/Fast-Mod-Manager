#pragma once

#include <cstdint>
#include <string>

namespace fmm::core {

enum class GamePluginErrorCode : std::uint8_t {
  FileNotFound,
  ScriptEvaluationFailed,
  InvalidFormat,
  MissingRequiredField
};

struct GamePluginError {
  GamePluginErrorCode code{GamePluginErrorCode::FileNotFound};
  std::string message;
};

} // namespace fmm::core
