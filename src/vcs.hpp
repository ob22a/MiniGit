

#ifndef VCS_HPP
#define VCS_HPP

#include <string>

namespace vcs {
    void init(); 
    void add(const std::string& filename);
    void commit(const std::string& message);
    void log();
    void branch(const std::string& branchName);
    void checkout(const std::string& target); // can be branch or commit
    void merge(const std::string& branchName);
    void diff(const std::string& hash1, const std::string& hash2);
};

#endif