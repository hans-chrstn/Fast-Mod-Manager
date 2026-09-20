#include "ui/AsyncTask.hpp"

#include <QCoreApplication>
#include <QTimer>
#include <catch2/catch_test_macros.hpp>
#include <chrono>

namespace {
auto process_events_until(const std::function<bool()>& condition, int timeout_ms = 1000) -> void {
  const int sleep_ms = 10;
  auto start = std::chrono::steady_clock::now();
  while (!condition()) {
    QCoreApplication::processEvents();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeout_ms) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
  }
}
} // namespace

TEST_CASE("AsyncTask executes work and returns result to UI thread", "[ui][async]") {
  std::unique_ptr<QCoreApplication> app;
  if (QCoreApplication::instance() == nullptr) {
    static int argc = 0;
    static std::array<char*, 1> argv = {nullptr};
    app = std::make_unique<QCoreApplication>(argc, argv.data());
  }

  QObject context;

  SECTION("Successfully completes work") {
    bool success_called = false;
    std::string result_val;

    fmm::ui::AsyncTask<std::string> task(
        &context, [](const std::stop_token&) -> std::string { return {"Success"}; },
        [&](const std::string& res) -> void {
          success_called = true;
          result_val = res;
        },
        [](const std::string&) -> void {});

    process_events_until([&]() -> bool { return success_called; });

    REQUIRE(success_called);
    REQUIRE(result_val == "Success");
  }

  SECTION("Handles exceptions gracefully") {
    bool error_called = false;
    std::string error_msg;

    fmm::ui::AsyncTask<std::string> task(
        &context,
        [](const std::stop_token&) -> std::string { throw std::runtime_error("Failure"); },
        [](const std::string&) -> void {},
        [&](const std::string& err) -> void {
          error_called = true;
          error_msg = err;
        });

    process_events_until([&]() -> bool { return error_called; });

    REQUIRE(error_called);
    REQUIRE(error_msg == "Failure");
  }

  SECTION("Cancellation stops execution and does not trigger callbacks") {
    bool success_called = false;
    bool error_called = false;
    const int cancel_timeout = 100;
    const int sleep_ms = 10;

    {
      fmm::ui::AsyncTask<std::string> task(
          &context,
          [sleep_ms](const std::stop_token& stoken) -> std::string {
            while (!stoken.stop_requested()) {
              std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            }
            return "Should not reach here";
          },
          [&](const std::string&) -> void { success_called = true; },
          [&](const std::string&) -> void { error_called = true; });
    }

    process_events_until([&]() -> bool { return success_called || error_called; }, cancel_timeout);

    REQUIRE_FALSE(success_called);
    REQUIRE_FALSE(error_called);
  }
}
