#pragma once

#include "core/IProcessLauncher.hpp"

namespace fmm::test_support {

class StubProcessLauncher : public core::IProcessLauncher {
public:
  [[nodiscard]] auto launch(const std::string& executable_path,
                            const std::vector<std::string>& arguments,
                            const std::string& working_directory) const
      -> std::expected<void, core::ProcessLauncherError> override;
};

} // namespace fmm::test_support
