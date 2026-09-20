#include "infrastructure/FilesystemGamePluginScanner.hpp"

#include <filesystem>

namespace fmm::infrastructure {

auto FilesystemGamePluginScanner::discoverPlugins(const std::string& directory) const
    -> std::vector<std::string> {
  std::vector<std::string> plugins;
  std::error_code error_code;

  if (std::filesystem::exists(directory, error_code) &&
      std::filesystem::is_directory(directory, error_code)) {
    for (const auto& entry : std::filesystem::directory_iterator(directory, error_code)) {
      if (entry.is_directory(error_code)) {
        auto init_path = entry.path() / "init.lua";
        if (std::filesystem::exists(init_path, error_code) &&
            std::filesystem::is_regular_file(init_path, error_code)) {
          plugins.push_back(init_path.string());
        }
      }
    }
  }

  return plugins;
}

} // namespace fmm::infrastructure
