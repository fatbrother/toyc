#pragma once

#include <string>
#include <vector>
#include <memory>

namespace toyc::parser {

int parseFile(const std::string &fileName);
int parseContent(const std::string &content);

} // namespace toyc::parser