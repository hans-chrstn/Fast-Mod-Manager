#include "application/InventoryService.hpp"
#include "ui/InventoryModel.hpp"

#include <QString>
#include <catch2/catch_test_macros.hpp>

namespace {

class StubInventoryService : public application::InventoryService {
public:
  [[nodiscard]] auto getInventory() const -> std::vector<fmm::domain::ModIdentity> override {
    return {fmm::domain::ModIdentity::create("Stub Mod 1", "/tmp/m1").value(),
            fmm::domain::ModIdentity::create("Stub Mod 2", "/tmp/m2").value()};
  }
};

} // namespace

TEST_CASE("InventoryModel populates from InventoryService", "[ui][inventory]") {
  auto service = std::make_shared<StubInventoryService>();
  ui::InventoryModel model(service);

  SECTION("row count matches service output") { REQUIRE(model.rowCount() == 2); }

  SECTION("data returns correctly mapped mod names") {
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
    REQUIRE_THROWS_AS(ui::InventoryModel(nullptr), std::invalid_argument);
  }
}
