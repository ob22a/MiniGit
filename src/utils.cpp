#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace utils {

    std::string readFile(const std::string& filepath) {
        std::ifstream file(filepath);
        std::stringstream buffer;
        if (file) {
            buffer << file.rdbuf();
            file.close();
        } else {
            return "";
        }
        return buffer.str();
    }

    std::vector<std::string> simulateDiff(const std::string& file1, const std::string& file2) {
        std::vector<std::string> result;

        std::string content1 = readFile(file1);
        std::string content2 = readFile(file2);

        std::istringstream stream1(content1);
        std::istringstream stream2(content2);

        std::string line1, line2;
        int lineNum = 1;

        while (std::getline(stream1, line1) && std::getline(stream2, line2)) {
            if (line1 != line2) {
                result.push_back("Line " + std::to_string(lineNum) + " differs");
            }
            lineNum++;
        }

        while (std::getline(stream1, line1)) {
            result.push_back("Line " + std::to_string(lineNum) + " only in file1");
            lineNum++;
        }

        while (std::getline(stream2, line2)) {
            result.push_back("Line " + std::to_string(lineNum) + " only in file2");
            lineNum++;
        }

        return result;
    }

    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }

    void log(const std::string& message) {
        std::cerr << "[LOG] " << message << std::endl;
    }
}


