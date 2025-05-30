#pragma once

#include <string>
#include <iostream>
#include <fstream>


namespace toyc::utility {

class ErrorHandler {
public:
    ErrorHandler(const std::string &message, int line = 0, int column = 0)
        : errorMessage(message), lineNumber(line), columnNumber(column) {}

    void setFileName(const std::string &name) {
        fileName = name;
    }

    void logError() const {
        std::string errorLine;

        if (!fileName.empty()) {
            std::ifstream file(fileName);
            if (file.is_open()) {
                std::string line;
                int currentLine = 0;
                while (std::getline(file, line)) {
                    currentLine++;
                    if (currentLine == lineNumber) {
                        errorLine = line;
                        break;
                    }
                }
                file.close();
            } else {
                std::cerr << "Error opening file: " << fileName << std::endl;
            }
        }

        std::cerr << errorLine << std::endl;
        for (int i = 0; i < columnNumber - 1; ++i) {
            std::cerr << " ";
        }
        std::cerr << "^" << std::endl;
        std::cerr << "Error: " << errorMessage << std::endl;
    }

private:
    std::string errorMessage;
    std::string fileName;
    int lineNumber = 0;
    int columnNumber = 0;
};

} // namespace toyc::utility