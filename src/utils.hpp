#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace utils {

    std::string readFile(const std::string& filepath);
    std::vector<std::string> simulateDiff(const std::string& file1, const std::string& file2);
    std::string trim(const std::string& str);
    void log(const std::string& message);
}

#endif // UTILS_HPP


