
#ifndef VCS_HPP
#define VCS_HPP

#include <string>
#include <vector>

class VCS {
public:
    VCS(); // constructor

    bool init();  // create .minigit structure
    bool add(const std::string& filename);
    bool commit(const std::string& message);
    void log() const;
    bool branch(const std::string& branchName);
    bool checkout(const std::string& name); // can be branch or commit
    bool merge(const std::string& branchName);

private:
    std::string getCurrentCommitHash() const;
    bool writeBlob(const std::string& content, const std::string& hash);
    bool updateIndex(const std::string& filename, const std::string& hash);
    std::string computeHash(const std::string& filepath);
};

#endif
