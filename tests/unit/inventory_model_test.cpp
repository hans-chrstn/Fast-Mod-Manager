#include "application/InventoryService.hpp"
#include "ui/InventoryModel.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QString>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <expected>
#include <functional>
#include <stop_token>
#include <string>
#include <thread>

namespace {

class StubInventoryService : public fmm::application::InventoryService {
public:
  [[nodiscard]] auto
  getInventory(const std::stop_token& /*stoken*/ = {},
               const std::function<void(int, const std::string&)>& /*progress_callback*/ = {}) const
      -> std::expected<std::vector<fmm::domain::InstalledPackage>,
                       fmm::application::InventoryError> override {
    return std::vector<fmm::domain::InstalledPackage>{
        fmm::domain::InstalledPackage(
            fmm::domain::PackageId("stub_1"),
            fmm::domain::PackageMetadata{
                .name = "Stub Mod 1", .version = "1.0", .source = "fixture"},
            fmm::domain::PackageLocation{.staging_path = "/tmp/m1",
                                         .object_store_ref = std::nullopt}),
        fmm::domain::InstalledPackage(
            fmm::domain::PackageId("stub_2"),
            fmm::domain::PackageMetadata{
                .name = "Stub Mod 2", .version = "1.0", .source = "fixture"},
            fmm::domain::PackageLocation{.staging_path = "/tmp/m2",
                                         .object_store_ref = std::nullopt})};
  }
};

void process_events_until(const std::function<bool()>& condition, int timeout_ms = 1000) {
  QElapsedTimer timer;
  timer.start();
  while (!condition() && timer.elapsed() < timeout_ms) {
    QCoreApplication::processEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

} // namespace

TEST_CASE("InventoryModel populates from InventoryService", "[ui][inventory]") {
  int argc = 1;
  char* argv[] = {const_cast<char*>("test")};
  QCoreApplication app(argc, argv);

  auto service = std::make_shared<StubInventoryService>();
  fmm::ui::InventoryModel model(service);

  SECTION("row count matches service output") {
    bool loaded = false;
    QObject::connect(&model, &fmm::ui::InventoryModel::scanCompleted, [&]() { loaded = true; });
    model.reload();
    process_events_until([&]() { return loaded; });

    REQUIRE(model.rowCount() == 2);
  }

  SECTION("data returns correctly mapped mod names") {
    bool loaded = false;
    QObject::connect(&model, &fmm::ui::InventoryModel::scanCompleted, [&]() { loaded = true; });
    model.reload();
    process_events_until([&]() { return loaded; });

    auto index_0 = model.index(0);
    auto index_1 = model.index(1);

    REQUIRE(index_0.isValid());
    REQUIRE(index_1.isValid());

    auto data_0 = model.data(index_0, Qt::DisplayRole);
    auto data_1 = model.data(index_1, Qt::DisplayRole);

    REQUIRE(data_0.toString() == QStringLiteral("Stub Mod 1"));
    REQUIRE(data_1.toString() == QStringLiteral("Stub Mod 2"));
  }

  SECTION("throws std::invalid_argument if service is null") {
    REQUIRE_THROWS_AS(fmm::ui::InventoryModel(nullptr), std::invalid_argument);
  }
}
