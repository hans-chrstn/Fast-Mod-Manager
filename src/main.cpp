#include "app/Bootstrapper.hpp"

auto main(int argc, char* argv[]) -> int {
  app::Bootstrapper bootstrapper(argc, argv);
  return bootstrapper.run();
}
