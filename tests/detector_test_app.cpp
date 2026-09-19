#include "infrastructure/LibArchivePackageDetector.hpp"

#include <iostream>
#include <system_error>

auto main(int argc, char** argv) -> int {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_archive>\n";
    return 1;
  }

  fmm::infrastructure::LibArchivePackageDetector detector;
  std::error_code error;
  auto identity = detector.detectFormat(argv[1], error);

  if (error) {
    std::cerr << "Error detecting format: " << error.message() << "\n";
    return 1;
  }

  std::cout << "Package Name: " << identity.name() << "\n";
  std::cout << "Detected Format: ";

  switch (identity.format()) {
  case fmm::domain::PackageFormat::Zip:
    std::cout << "ZIP\n";
    break;
  case fmm::domain::PackageFormat::SevenZip:
    std::cout << "7z\n";
    break;
  case fmm::domain::PackageFormat::Rar:
    std::cout << "RAR\n";
    break;
  case fmm::domain::PackageFormat::Unknown:
    std::cout << "Unknown\n";
    break;
  }

  return 0;
}
