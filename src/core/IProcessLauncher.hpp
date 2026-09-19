#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace core {

enum class ProcessLauncherError : std::uint8_t {
  ExecutableNotFound,
  PermissionDenied,
  ExecutionFailed
};

class IProcessLauncher {
public:
  virtual ~IProcessLauncher() = default;

  [[nodiscard]] virtual auto launch(const std::string& executable_path,
                                    const std::vector<std::string>& arguments,
                                    const std::string& working_directory) const
      -> std::expected<void, ProcessLauncherError> = 0;
};

} // namespace core
