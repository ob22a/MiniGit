#include "utils.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <iomanip>

namespace utils {

    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }

    void displayError(const std::string& message) {
        std::cerr << message << std::endl;
    }

    void showDiff(const std::string& content1, const std::string& content2,
                  const std::string& label1, const std::string& label2) {
        std::istringstream stream1(content1);
        std::istringstream stream2(content2);

        std::string line1, line2;
        std::vector<std::string> lines1, lines2;

        while (std::getline(stream1, line1)) lines1.push_back(line1);
        while (std::getline(stream2, line2)) lines2.push_back(line2);

        size_t maxLines = std::max(lines1.size(), lines2.size());

        std::cout << " Diff between " << label1 << " and " << label2 << ":\n\n";

        for (size_t i = 0; i < maxLines; ++i) {
            std::string l1 = (i < lines1.size()) ? lines1[i] : "";
            std::string l2 = (i < lines2.size()) ? lines2[i] : "";

            if (l1 != l2) {
                std::cout << "\033[1;31m- " << i + 1 << "    " << l1 << "\033[0m\n"; // red
                std::cout << "\033[1;32m+ " << i + 1 << "    " << l2 << "\033[0m\n"; // green
            }
        }
    }
}




