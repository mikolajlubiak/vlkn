#include "vlkn_pipeline.hpp"

#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace vlkn {

std::vector<char> VlknPipeline::readFile(const std::string &path) {
  std::ifstream file{path, std::ios::ate | std::ios::binary};

  if (!file.is_open() || !file.good()) {
    throw std::runtime_error("failed to open file: " + path);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

void VlknPipeline::createGraphicsPipeline(const std::string &vert,
                                          const std::string &frag) {
  std::vector<char> vertCode = readFile(vert);
  std::vector<char> fragCode = readFile(frag);

  std::cout << vertCode.size() << '\n';
  std::cout << fragCode.size() << '\n';
}

VlknPipeline::VlknPipeline(const std::string &vert, const std::string &frag) {
  createGraphicsPipeline(vert, frag);
}

} // namespace vlkn
