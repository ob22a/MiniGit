#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace utils {

    std::string runCommand(const std::string& command);
    std::vector<std::string> diffCommits(const std::string& commit1, const std::string& commit2);
    std::string trim(const std::string& str);
    void log(const std::string& message);
}

#endif // UTILS_HPP

