#include <QApplication>
#include <QLabel>
#include <QMainWindow>

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);
  QMainWindow window;
  QLabel* label = new QLabel(QStringLiteral("Ready"), &window);
  label->setAlignment(Qt::AlignCenter);
  window.setCentralWidget(label);
  window.setWindowTitle(QStringLiteral("Mod Manager"));
  window.resize(960, 640);
  window.show();
  return application.exec();
}
