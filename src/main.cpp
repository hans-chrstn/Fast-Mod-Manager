#include "app/Bootstrapper.hpp"
#include "application/CatalogInventoryService.hpp"
#include "application/ports/IPackageCatalog.hpp"
#include "infrastructure/JsonPackageCatalog.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
auto catalogPath() -> std::filesystem::path {
  const auto application_data = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (application_data.isEmpty()) {
    throw std::runtime_error("Application data directory is unavailable");
  }

  const auto encoded_path = QFile::encodeName(application_data + "/packages.json");
  return {std::string(encoded_path.constData(), static_cast<std::size_t>(encoded_path.size()))};
}
} // namespace

auto main(int argc, char* argv[]) -> int {
  try {
    QCoreApplication::setOrganizationName(QStringLiteral("FastModManager"));
    QCoreApplication::setApplicationName(QStringLiteral("Fast Mod Manager"));
    const auto catalog_file = catalogPath();

    fmm::app::Bootstrapper bootstrapper(
        argc, argv, [catalog_file](fmm::core::ServiceRegistry& registry) -> void {
          registry.registerService<fmm::application::ports::IPackageCatalog>(
              std::make_shared<fmm::infrastructure::JsonPackageCatalog>(catalog_file));

          registry.registerService<fmm::application::InventoryService>(
              std::make_shared<fmm::application::CatalogInventoryService>(
                  registry.resolve<fmm::application::ports::IPackageCatalog>()));
        });
    return bootstrapper.run();
  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "Fatal unknown error\n";
    return 1;
  }
}
