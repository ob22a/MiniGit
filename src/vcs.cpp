#include "vcs.hpp"
#include "dsa.hpp"
#include "io.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <chrono>
using ::writeBlob;

namespace fs = std::filesystem;

std::string VCS::computeHash(const std::string& content) {
    return computeSHA1(content); // provided by dsa
}


VCS::VCS() {}

bool VCS::init() {
    try {
        fs::create_directory(".minigit");
        fs::create_directory(".minigit/objects");
        fs::create_directory(".minigit/commits");
        fs::create_directory(".minigit/refs");
        fs::create_directory(".minigit/refs/heads");

        std::ofstream head(".minigit/HEAD");
        head << "ref: refs/heads/main\n";
        head.close();

        std::ofstream index(".minigit/index");
        index.close();

        std::ofstream branch(".minigit/refs/heads/main");
        branch.close();

        std::cout << "Initialized empty MiniGit repository in .minigit/\n";
        return true;
    } catch (...) {
        std::cerr << "Error: Failed to initialize repository.\n";
        return false;
    }
}

std::string VCS::getFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}




bool VCS::updateIndex(const std::string& filename, const std::string& hash) {
    std::ifstream indexIn(".minigit/index");
    std::stringstream updatedIndex;
    std::string line;
    bool updated = false;

    while (std::getline(indexIn, line)) {
        std::istringstream iss(line);
        std::string existingFile, existingHash;
        iss >> existingFile >> existingHash;

        if (existingFile == filename) {
            updatedIndex << filename << " " << hash << "\n";
            updated = true;
        } else {
            updatedIndex << line << "\n";
        }
    }

    if (!updated) {
        updatedIndex << filename << " " << hash << "\n";
    }

    std::ofstream indexOut(".minigit/index");
    indexOut << updatedIndex.str();
    return true;
}

bool VCS::add(const std::string& filename) {
    if (!fs::exists(filename)) {
        std::cerr << "Error: File does not exist.\n";
        return false;
    }

    std::string content = getFileContent(filename);
    std::string hash = computeHash(content);

    if (!writeBlob(hash, content)) return false;
    if (!updateIndex(filename, hash)) return false;

    std::cout << "Added " << filename << " to staging area.\n";
    return true;
}

bool VCS::commit(const std::string& message) {
    std::ifstream index(".minigit/index");
    if (!index) {
        std::cerr << "Error: Could not open index.\n";
        return false;
    }

    std::string stagedContent((std::istreambuf_iterator<char>(index)),
                              std::istreambuf_iterator<char>());
    if (stagedContent.empty()) {
        std::cerr << "Nothing to commit.\n";
        return false;
    }

    std::string parent = getCurrentCommitHash();

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);

    std::stringstream commitData;
    commitData << "parent: " << parent << "\n";
    commitData << "timestamp: " << std::ctime(&timestamp);
    commitData << "message: " << message << "\n";
    commitData << "files:\n" << stagedContent;

    std::string commitHash = computeHash(commitData.str());
    std::ofstream commitFile(".minigit/commits/" + commitHash);
    commitFile << commitData.str();
    commitFile.close();

    std::ofstream head(".minigit/HEAD");
    head << commitHash;
    head.close();

    std::cout << "Committed as " << commitHash << "\n";
    return true;
}

std::string VCS::getCurrentCommitHash() const {
    std::ifstream head(".minigit/HEAD");
    std::string line;
    std::getline(head, line);

    if (line.rfind("ref:", 0) == 0) {
        std::string refPath = ".minigit/" + line.substr(5);
        std::ifstream refFile(refPath);
        std::string commit;
        std::getline(refFile, commit);
        return commit;
    } else {
        return line; // detached HEAD
    }
}

void VCS::log() const {
    std::string commitHash = getCurrentCommitHash();
    while (!commitHash.empty()) {
        std::ifstream commitFile(".minigit/commits/" + commitHash);
        if (!commitFile) break;

        std::string line;
        std::string nextHash;
        std::cout << "Commit: " << commitHash << "\n";
        while (std::getline(commitFile, line)) {
            if (line.rfind("parent:", 0) == 0) {
                nextHash = line.substr(8);
            }
            std::cout << line << "\n";
        }
        std::cout << "------------------\n";

        if (nextHash.empty() || nextHash == commitHash) break;
        commitHash = nextHash;
    }
}


bool VCS::branch(const std::string& branchName) {
    std::string branchPath = ".minigit/refs/heads/" + branchName;

    if (fs::exists(branchPath)) {
        std::cerr << "Branch already exists.\n";
        return false;
    }

    std::string currentHash = getCurrentCommitHash();
    std::ofstream branchFile(branchPath);
    branchFile << currentHash;
    branchFile.close();

    std::cout << "Branch '" << branchName << "' created at " << currentHash << "\n";
    return true;
}


bool VCS::checkout(const std::string& name) {
    std::string branchPath = ".minigit/refs/heads/" + name;
    std::string commitHash;

    if (fs::exists(branchPath)) {
        std::ifstream branchFile(branchPath);
        std::getline(branchFile, commitHash);

        std::ofstream head(".minigit/HEAD");
        head << "ref: refs/heads/" << name << "\n";
        head.close();

        std::cout << "Switched to branch '" << name << "'\n";
    } else {
        // Detached HEAD
        commitHash = name;

        if (!fs::exists(".minigit/commits/" + commitHash)) {
            std::cerr << "Invalid commit hash.\n";
            return false;
        }

        std::ofstream head(".minigit/HEAD");
        head << commitHash << "\n";
        head.close();

        std::cout << "Detached HEAD at " << commitHash << "\n";
    }

    // Restore files from commit
    std::ifstream commit(".minigit/commits/" + commitHash);
    std::string line;
    bool inFiles = false;

    while (std::getline(commit, line)) {
        if (line == "files:") {
            inFiles = true;
            continue;
        }

        if (inFiles) {
            std::istringstream iss(line);
            std::string filename, hash;
            iss >> filename >> hash;

            std::string blobPath = ".minigit/objects/" + hash.substr(0, 2) + "/" + hash.substr(2);
            std::ifstream blob(blobPath);
            std::ofstream outFile(filename);
            outFile << blob.rdbuf();
        }
    }

    return true;
}



bool VCS::merge(const std::string& branchName) {
    std::string branchPath = ".minigit/refs/heads/" + branchName;
    if (!fs::exists(branchPath)) {
        std::cerr << "Branch not found.\n";
        return false;
    }

    std::string theirCommitHash;
    std::ifstream branchFile(branchPath);
    std::getline(branchFile, theirCommitHash);

    std::string ourCommitHash = getCurrentCommitHash();

    std::string lca = findLCA(ourCommitHash, theirCommitHash);

    std::cout << "Merging branch '" << branchName << "' into current branch.\n";

    // Simulated conflict
    std::string conflictedFile = "example.txt";
    std::ofstream file(conflictedFile);
    file << "<<<<<<< HEAD\n";
    file << "our version\n";
    file << "=======\n";
    file << "their version\n";
    file << ">>>>>>> " << branchName << "\n";
    file.close();

    std::cout << "Merge conflict in " << conflictedFile << "\n";
    return true;
}   
