#include "vcs.hpp"
#include "io.hpp"
#include "dsa.hpp"
#include "utils.hpp"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <set>

namespace fs = std::filesystem;

namespace vcs{

void init() {
    if(IOManager::fileExists(IOManager::MINIGIT_DIR)){
        utils::displayError("MiniGit repo already exists.\n");
        return;
    }
    if(!IOManager::initMinigitDir()){
        utils::displayError("Failed to create .minigit .\n");
        return;
    }
    std::cout << "Initialized empty MiniGit repository.\n";
}

void add(const std::string& filename) {
    if (!IOManager::fileExists(filename)) {
        utils::displayError("Error: File does not exist.\n");
        return;
    }

    std::string content = IOManager::readFile(filename);
    std::string hash = IOManager::writeBlob(content); // also computes SHA1

    auto staged = IOManager::readIndex();
    bool updated = false;
    for (auto& pair : staged) {
        if (pair.first == filename) {
            pair.second = hash;
            updated = true;
        }
    }
    if (!updated) {
        staged.emplace_back(filename, hash);
    }
    IOManager::updateIndex(staged);

    std::cout << "Staged file: " << filename << " (" << hash.substr(0, 7) << ")\n";
}

void commit(const std::string& message) {
    auto staged = IOManager::readIndex();
    if (staged.empty()) {
        std::cout << "Nothing to commit.\n";
        return;
    }

    std::stringstream data;
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timestamp = std::ctime(&now);
    timestamp.pop_back();

    data << "timestamp: " << timestamp << "\n";
    data << "message: " << message << "\n";

    std::string parent = IOManager::resolveHEAD();
    if (!parent.empty()) data << "parent: " << parent << "\n";

    for (const auto& [filename, hash] : staged) {
        data << "file: " << filename << " " << hash << "\n";
    }

    std::string commitHash = dsa::computeSHA1(data.str());
    dsa::addCommit(commitHash, parent.empty() ? std::vector<std::string>{} : std::vector<std::string>{parent});
    IOManager::writeCommit(commitHash, data.str());

    // Update HEAD reference
    if (!IOManager::fileExists(IOManager::HEAD_FILE)) {
        utils::displayError("HEAD file is missing. Cannot update HEAD.\n");
        return;
    }

    std::string head = IOManager::readFile(IOManager::HEAD_FILE);
    if (head.rfind("ref:", 0) == 0) {
        // head is "ref: refs/heads/main\n"
        std::string ref = head.substr(5); // yields "refs/heads/main\n"
        // Remove the prefix "refs/heads/" (which is 11 characters) and trim the result.
        std::string branchName = utils::trim(ref.substr(11)); // yields "main"
        IOManager::writeReference(branchName, commitHash);
    } else {
        IOManager::writeFile(IOManager::HEAD_FILE, commitHash); // detached HEAD
    }

    IOManager::updateIndex({});
    std::cout << "Committed as " << commitHash.substr(0, 7) << ": " << message << "\n";
}

void log() {
    std::string current = IOManager::resolveHEAD();
    if (current.empty()) { // No commit found in HEAD
        std::cout << "No commits yet.\n";
        return;
    }

    std::string headContent = IOManager::readFile(IOManager::HEAD_FILE);
    if(headContent.rfind("ref:", 0)==0) {
        std::string branch = utils::trim(headContent.substr(11)); // Assuming "refs/heads/<branch>"
        std::cout << "On branch: " << branch << "\n\n";
    }

    while (!current.empty()) {
        std::string commitData = IOManager::readCommit(current);
        if (commitData.empty()) {
            utils::displayError("Commit data is missing or corrupted for commit: " + current + "\n");
            break;
        }

        std::cout << "Commit " << current << "\n";

        std::istringstream iss(commitData);
        std::string line;
        std::string nextParent;


        while (std::getline(iss, line)) {
            if (line.rfind("message:", 0) == 0 || line.rfind("timestamp:", 0) == 0)
                std::cout << "   " << line << "\n";
            if (line.rfind("parent:", 0) == 0)
                nextParent = line.substr(8);
        }

        if (nextParent.empty()) break;
        current = nextParent;
    }
}


void branch(const std::string& branchName) {
    std::string headCommit = IOManager::resolveHEAD();
    if (headCommit.empty()) {
        std::cerr << "No commit to branch from.\n";
        return;
    }
    IOManager::writeReference(branchName, headCommit);
    std::cout << "Created branch '" << branchName << "' at " << headCommit.substr(0, 7) << "\n";
}


void checkout(const std::string& target) {
    // Get current HEAD commit (old commit) before switching
    std::string oldCommit = IOManager::resolveHEAD();
    std::set<std::string> oldFiles;
    if (!oldCommit.empty()) {
        std::string oldCommitData = IOManager::readCommit(oldCommit);
        std::istringstream oldIss(oldCommitData);
        std::string line;
        while (std::getline(oldIss, line)) {
            if (line.rfind("file: ", 0) == 0) {
                size_t pos = line.find(' ', 6);
                if (pos != std::string::npos) {
                    std::string filename = line.substr(6, pos - 6);
                    oldFiles.insert(filename);
                }
            }
        }
    }

    // Try to read target as a branch reference.
    std::string commitHash = IOManager::readReference(target);
    if (!commitHash.empty()) {
        // Update HEAD file to point to this branch.
        IOManager::writeFile(IOManager::HEAD_FILE, "ref: refs/heads/" + target);
        std::cout << "Switched to branch '" << target << "'\n";

        // Retrieve commit data for the new branch.
        std::string commitData = IOManager::readCommit(commitHash);
        std::set<std::string> newFiles;
        if (!commitData.empty()) {
            std::istringstream iss(commitData);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.rfind("file: ", 0) == 0) {
                    size_t pos = line.find(' ', 6);
                    if (pos != std::string::npos) {
                        std::string filename = line.substr(6, pos - 6);
                        newFiles.insert(filename);
                        std::string blobHash = line.substr(pos + 1);
                        std::string content = IOManager::readBlob(blobHash);
                        if (!content.empty()) {
                            IOManager::writeFile(filename, content);
                        }
                    }
                }
            }
        }
        // Remove any file that was tracked in the previous commit but is not present in the new commit.

        for (const auto &file : oldFiles) {
            if (newFiles.find(file) == newFiles.end() && IOManager::fileExists(file)) {
                fs::remove(file);
                std::cout << "Removed file: " << file << "\n";
            }
        }
        return;
    }

    // If no branch reference is found, try treating target as a commit hash (detached HEAD).
    std::string commitData = IOManager::readCommit(target);
    if (commitData.empty()) {
        utils::displayError("Invalid branch or commit.\n");
        return;
    }
    IOManager::writeFile(IOManager::HEAD_FILE, target); // detached HEAD
    std::cout << "Checked out commit " << target.substr(0, 7) << " (detached HEAD)\n";
}


void merge(const std::string& branchName) {
    std::string headCommit = IOManager::resolveHEAD();
    std::string otherCommit = IOManager::readReference(branchName);
    if (otherCommit.empty()) {
        utils::displayError("Branch not found.\n");
        return;
    }
    
    std::string lca = dsa::findLCA(headCommit, otherCommit);
    std::cout << "Merging branch '" << branchName << "'\n";
    std::cout << "LCA: " << (lca.empty() ? "none" : lca.substr(0, 7)) << "\n";



    if (lca == otherCommit) {
        std::cout << "Branch '" << branchName << "' is already merged.\n";
        return;
    }
    if (lca == headCommit) {
        std::cout << "Fast-forwarding to branch '" << branchName << "'.\n";
        checkout(branchName);
        std::cout << "Working directory updated to match branch '" << branchName << "'.\n";
        return;
    }

    // Parse commit files from each commit using a lambda.
    auto getCommitFiles = [](const std::string& commitHash) -> std::unordered_map<std::string, std::string> {
        std::unordered_map<std::string, std::string> files;
        std::string commitData = IOManager::readCommit(commitHash);
        std::istringstream iss(commitData);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.compare(0, 6, "file: ") == 0) {
                size_t pos = line.find(' ', 6);
                if (pos != std::string::npos) {
                    std::string filename = line.substr(6, pos - 6);
                    std::string hash = line.substr(pos + 1);
                    files[filename] = hash;
                }
            }
        }
        return files;
    };

    auto baseFiles  = getCommitFiles(lca);
    auto headFiles  = getCommitFiles(headCommit);
    auto otherFiles = getCommitFiles(otherCommit);

    // Create a set of all filenames present in either the HEAD or other branch.
    std::set<std::string> allFiles;
    for (const auto& [filename, _] : headFiles)
        allFiles.insert(filename);
    for (const auto& [filename, _] : otherFiles)
        allFiles.insert(filename);

    std::unordered_map<std::string, std::string> mergedFiles;
    bool conflict = false;
    std::vector<std::string> conflictFiles;

    for (const auto& filename : allFiles) {
        // Get versions from each commit; if missing, use empty string.
        std::string baseHash  = (baseFiles.find(filename)  != baseFiles.end()) ? baseFiles[filename]  : "";
        std::string headHash  = (headFiles.find(filename)  != headFiles.end()) ? headFiles[filename]  : "";
        std::string otherHash = (otherFiles.find(filename) != otherFiles.end()) ? otherFiles[filename] : "";
        
        // If both branches did not change (or they match), use either version.
        if (headHash == otherHash) {
            mergedFiles[filename] = headHash; // No conflict
        }
        // If current branch didn't change relative to base, then adopt the other branch change.
        else if (!baseHash.empty() && headHash == baseHash && !otherHash.empty()) {
            mergedFiles[filename] = otherHash;
        }
        // If other branch didn't change relative to base, adopt the current branch change.
        else if (!baseHash.empty() && otherHash == baseHash && !headHash.empty()) {
            mergedFiles[filename] = headHash;
        }
        // Otherwise, if modifications exist in both that differ from base, mark a conflict.
        else {
            conflict = true;
            conflictFiles.push_back(filename);
            std::string headContent  = headHash.empty()  ? "" : IOManager::readBlob(headHash);
            std::string otherContent = otherHash.empty() ? "" : IOManager::readBlob(otherHash);
            mergedFiles[filename] = "<<<<<<< HEAD\n" + headContent +
                                    "=======\n" + otherContent +
                                    ">>>>>>>\n";
        }
    }

    if (conflict) {
        std::cout << "Merge completed with conflicts in the following files:\n";
        for (const auto& file : conflictFiles) {
            std::cout << " - " << file << "\n";
        }
        std::cout << "Resolve conflicts and commit the result.\n";
    } else {
        std::cout << "Merge completed successfully.\n";
    }



    // Write merged content to working directory.
    for (const auto& [filename, blobHashOrContent] : mergedFiles) {
        // If the merge produced conflict markers, blobHashOrContent is the final content.
        // Otherwise, we fetch the blob content.
        std::string finalContent;
        if (conflict && std::find(conflictFiles.begin(), conflictFiles.end(), filename) != conflictFiles.end()) {
            finalContent = blobHashOrContent;
        } else {
            finalContent = IOManager::readBlob(blobHashOrContent);
            if (finalContent.empty()) { // In case we already stored a blob hash.
                finalContent = blobHashOrContent; 
            }
        }
        if (IOManager::fileExists(filename)) {
            std::cout << "Overwriting file: " << filename << "\n";
        }
        IOManager::writeFile(filename, finalContent);
    }

    std::cout << "Merged changes into the working directory.\n";

    // Optionally, create a merge commit with a merge message.
    std::string mergeMessage = "Merge branch '" + branchName + "' into current branch";
    commit(mergeMessage);
}

void diff(const std::string& hash1, const std::string& hash2) {
    // Read commit data for each commit hash.
    std::string data1 = IOManager::readCommit(hash1);
    std::string data2 = IOManager::readCommit(hash2);

    if (data1.empty()) {
        utils::displayError("Commit " + hash1 + " not found or empty.\n");
        return;
    }
    if (data2.empty()) {
        utils::displayError("Commit " + hash2 + " not found or empty.\n");
        return;
    }

    // Helper lambda to parse commit file entries.
    auto parseCommitFiles = [](const std::string &commitData) -> std::unordered_map<std::string, std::string> {
        std::unordered_map<std::string, std::string> files;
        std::istringstream iss(commitData);
        std::string line;
        while (std::getline(iss, line)) {
            // We expect lines like: "file: <filename> <blobHash>"
            if (line.rfind("file: ", 0) == 0) {
                size_t pos = line.find(' ', 6);
                if (pos != std::string::npos) {
                    std::string filename = line.substr(6, pos - 6);
                    std::string blobHash = line.substr(pos + 1);
                    files[filename] = blobHash;
                }
            }
        }
        return files;
    };

    auto commitFiles1 = parseCommitFiles(data1);
    auto commitFiles2 = parseCommitFiles(data2);

    // Compute union of filenames.
    std::set<std::string> allFiles;
    for (const auto& [filename, _] : commitFiles1) {
        allFiles.insert(filename);
    }
    for (const auto& [filename, _] : commitFiles2) {
        allFiles.insert(filename);
    }

    // For each file in union, compare contents.
    for (const auto& filename : allFiles) {
        std::string blobHash1 = "";
        std::string blobHash2 = "";
        if (commitFiles1.find(filename) != commitFiles1.end())
            blobHash1 = commitFiles1[filename];
        if (commitFiles2.find(filename) != commitFiles2.end())
            blobHash2 = commitFiles2[filename];

        std::string content1 = blobHash1.empty() ? "" : IOManager::readBlob(blobHash1);
        std::string content2 = blobHash2.empty() ? "" : IOManager::readBlob(blobHash2);

        // Only show diff if contents differ.
        if (content1 != content2) {
            std::cout << "\nDiff for file: " << filename << "\n";
            utils::showDiff(content1, content2, hash1 + ":" + filename, hash2 + ":" + filename);
        }
    }
}
}

