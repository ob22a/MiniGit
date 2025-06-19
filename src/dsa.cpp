#include "dsa.hpp"
#include <sstream>
#include<string>
#include<vector>
#include <iomanip>
#include <unordered_set>
#include <queue>
#include <iostream>

namespace dsa{
static std::unordered_map<std::string, std::vector<std::string>> commitGraph; // Commit's hash -> list of Parent's hash( Directed Acyclic graph)

std::string computeSHA1(const std::string& content) {
    std::hash<std::string> hasher;
    size_t hashValue = hasher(content);

    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hashValue; //converts to hexadecimal with a length of 16 and adds 0 padding if necessary
    return ss.str();
}

void addCommit(const std::string& commitHash, const std::vector<std::string>& parentHashes) {
    commitGraph[commitHash] = parentHashes;
}

std::vector<std::string> getParents(const std::string& commitHash) {
    auto it = commitGraph.find(commitHash);
    return (it != commitGraph.end()) ? it->second : std::vector<std::string>{};
}

std::string findLCA(const std::string& commitA, const std::string& commitB) {
    std::unordered_set<std::string> visitedA, visitedB;
    std::queue<std::string> queueA, queueB;

    if (commitA.empty() || commitB.empty()) return "";

    queueA.push(commitA);
    queueB.push(commitB);

    while (!queueA.empty() || !queueB.empty()) {
        if (!queueA.empty()) {
            std::string current = queueA.front(); queueA.pop();
            if (visitedB.count(current)) return current;
            visitedA.insert(current);
            for (const auto& parent : getParents(current)) {
                if (!visitedA.count(parent)) queueA.push(parent);
            }
        }

        if (!queueB.empty()) {
            std::string current = queueB.front(); queueB.pop();
            if (visitedA.count(current)) return current;
            visitedB.insert(current);
            for (const auto& parent : getParents(current)) {
                if (!visitedB.count(parent)) queueB.push(parent);
            }
        }
    }

    return ""; // No common ancestor found
}

//For debugging 
void printCommitGraph() {
    std::cout << "\nCommit DAG:\n";
    for (const auto& [child, parents] : commitGraph) {
        std::cout<< child << " : ";
        for (const auto& p : parents) std::cout << p << " ";
        std::cout << "\n";
    }
    std::cout << std::endl;
}
}
