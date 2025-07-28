#pragma once

#include <string>
#include <iostream>

namespace toyc::utility {

class ErrorHandler {
public:
    ErrorHandler(const std::string &message, int line = 0, int column = 0, int tokenSize = 0)
        : errorMessage(message), lineNumber(line), columnNumber(column), tokenSize(tokenSize) {}
    void setFileName(const std::string &name);
    void logError() const;
    void logError(std::ostream& output) const;
    std::string getFormattedError() const;

    const std::string& getMessage() const { return errorMessage; }
    const std::string& getFileName() const { return fileName; }
    int getLineNumber() const { return lineNumber; }
    int getColumnNumber() const { return columnNumber; }

private:
    std::string errorMessage;
    std::string fileName;
    int lineNumber = 0;
    int columnNumber = 0;
    int tokenSize = 0;

    std::string buildIndicatorLine(const std::string& line, int logicalColumn) const;
};

} // namespace toyc::utility