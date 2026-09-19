#include "ui/AsyncTask.hpp"

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <chrono>
#include <memory>
#include <string>

auto main(int argc, char* argv[]) -> int {
  QApplication app(argc, argv);
  QWidget window;
  window.setWindowTitle("AsyncTask Test App");
  const int width = 300;
  const int height = 200;
  window.resize(width, height);

  auto* layout = new QVBoxLayout(&window);
  auto* startButton = new QPushButton("Start Long Task", &window);
  layout->addWidget(startButton);

  std::unique_ptr<ui::AsyncTask<std::string>> activeTask;

  QObject::connect(startButton, &QPushButton::clicked, [&]() -> void {
    startButton->setEnabled(false);
    startButton->setText("Running...");

    const int iterations = 50;
    const int sleep_ms = 100;

    activeTask = std::make_unique<ui::AsyncTask<std::string>>(
        &window,
        [iterations, sleep_ms](const std::stop_token& stoken) -> std::string {
          for (int i = 0; i < iterations; ++i) {
            if (stoken.stop_requested()) {
              return "Cancelled";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
          }
          return "Task Completed";
        },
        [startButton, &activeTask](const std::string& result) -> void {
          startButton->setEnabled(true);
          startButton->setText(QString::fromStdString(result));
          activeTask.reset();
        },
        [startButton, &activeTask](const std::string& err) -> void {
          startButton->setEnabled(true);
          startButton->setText(QString::fromStdString("Error: " + err));
          activeTask.reset();
        });
  });

  window.show();
  return QApplication::exec();
}
