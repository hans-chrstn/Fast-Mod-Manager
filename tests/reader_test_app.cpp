#include "infrastructure/LibArchiveReader.hpp"

#include <iostream>
#include <string>

void printNode(const fmm::domain::VirtualNode& node, int indent = 0) {
  for (int i = 0; i < indent; ++i) {
    std::cout << "  ";
  }
  std::cout << "- " << node.name;
  if (node.is_directory) {
    std::cout << "/\n";
  } else {
    std::cout << " (" << node.size << " bytes)\n";
  }

  for (const auto& child : node.children) {
    printNode(child, indent + 1);
  }
}

auto main(int argc, char** argv) -> int {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_archive>\n";
    return 1;
  }

  fmm::infrastructure::LibArchiveReader reader;
  auto result = reader.readTree(argv[1]);

  if (!result.has_value()) {
    std::cerr << "Error reading archive tree: ";
    switch (result.error()) {
    case fmm::core::ArchiveError::IoError:
      std::cerr << "I/O Error\n";
      break;
    case fmm::core::ArchiveError::InvalidFormat:
      std::cerr << "Invalid Format\n";
      break;
    case fmm::core::ArchiveError::CorruptHeader:
      std::cerr << "Corrupt Header\n";
      break;
    case fmm::core::ArchiveError::UnsupportedFilter:
      std::cerr << "Unsupported Filter\n";
      break;
    }
    return 1;
  }

  std::cout << "Archive Tree for: " << argv[1] << "\n";
  for (const auto& child : result.value().root().children) {
    printNode(child, 0);
  }

  return 0;
}
