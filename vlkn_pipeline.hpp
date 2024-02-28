#pragma once

#include <string>
#include <vector>

namespace vlkn {

class VlknPipeline {
public:
  VlknPipeline(const std::string &vert, const std::string &frag);

private:
  static std::vector<char> readFile(const std::string &path);
  void createGraphicsPipeline(const std::string &vert, const std::string &frag);
};

} // namespace vlkn
