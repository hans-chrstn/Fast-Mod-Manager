#pragma once

#include <QString>
#include <string>
#include <vector>

namespace core {

struct ExternalTool {
  std::string id;
  std::string name;
  std::string default_executable_path;
};

class IGameAdapter {
public:
  virtual ~IGameAdapter() = default;

  [[nodiscard]] virtual auto gameId() const -> std::string = 0;
  [[nodiscard]] virtual auto gameName() const -> QString = 0;
  [[nodiscard]] virtual auto defaultDataPath() const -> std::string = 0;
  [[nodiscard]] virtual auto knownTools() const -> std::vector<ExternalTool> = 0;
};

} // namespace core
