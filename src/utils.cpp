#include "utils.hpp"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>

namespace utils {

    std::string runCommand(const std::string& command) {
        std::string result;
        char buffer[128];
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) return "ERROR";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        return result;
    }

    std::vector<std::string> diffCommits(const std::string& commit1, const std::string& commit2) {
        std::string command = "git diff --name-only " + commit1 + " " + commit2;
        std::string output = runCommand(command);
        std::istringstream stream(output);
        std::vector<std::string> files;
        std::string line;

        while (std::getline(stream, line)) {
            files.push_back(trim(line));
        }

        return files;
    }

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    void log(const std::string& message) {
        std::cerr << "[UTILS LOG] " << message << std::endl;
    }
}

