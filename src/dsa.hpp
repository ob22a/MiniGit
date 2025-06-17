#ifndef DSA_HPP
#define DSA_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <set>

namespace dsa {

// SHA-1 Hashing
std::string computeSHA1(const std::string& content);

// DAG Management
void addCommit(const std::string& commitHash, const std::vector<std::string>& parentHashes);
std::vector<std::string> getParents(const std::string& commitHash);

// Longest Common Ancestor(LCA) Detection
std::string findLCA(const std::string& commitA, const std::string& commitB);

// For Debugging 
void printCommitGraph();

}
#endif
