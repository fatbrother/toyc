#pragma once

#include <string>
#include <vector>
#include <memory>

namespace toyc::parser {

int parseFile(const std::string &fileName);
int parseContent(const std::string &content);
int parseFileWithPreprocessor(const std::string &fileName,
                              const std::vector<std::pair<std::string, std::string>>& macros = {},
                              const std::vector<std::string>& includePaths = {});

} // namespace toyc::parser