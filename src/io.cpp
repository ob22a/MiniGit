#include "io.hpp"
#include "dsa.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream>
#include<filesystem>
#include<string>
#include <iostream>
#include <ctime>

using namespace std;
namespace fs = filesystem;

// Directory Operations

bool IOManager::initMinigitDir() {
    try {
        // Create repository structure
        fs::create_directory(MINIGIT_DIR);
        fs::create_directory(OBJECTS_DIR);
        fs::create_directories(COMMITS_DIR);
        fs::create_directories(REFS_HEADS_DIR);

        writeReference("main","");

        // Initialize key files
        writeFile(HEAD_FILE, "ref: refs/heads/main\n");
        writeFile(INDEX_FILE, "");

        return true;
    } catch (const exception &e) {
        utils::displayError(string("Error initializing repository: ")+e.what());
        return false;
    }
}

bool IOManager::createDir(const string &path) {
    return fs::create_directories(path);
}

// File Operations

string IOManager::readFile(const string &path) {
    ifstream file(path, ios::binary);
    if (!file) {
        utils::displayError(string("Cannot open file ")+path);
        return "";
    }
    
    return string(istreambuf_iterator<char>(file), 
                 istreambuf_iterator<char>());
}

bool IOManager::writeFile(const string &path, const string &content) {
    ofstream file(path, ios::binary);
    if (!file) {
        utils::displayError(string("Error writing to: ")+path);
        return false;
    }
    
    file << content;
    return file.good();
}

bool IOManager::copyFile(const string &src, const string &dest) {
    try {
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
        return true;
    } catch (const exception &e) {
        utils::displayError(string("Copy failed: ")+e.what());
        return false;
    }
}

bool IOManager::fileExists(const string &path) {
    return fs::exists(path);
}


// Blob Storage
string IOManager::writeBlob(const string &content) {
    const string hash = dsa::computeSHA1(content);
    const string dir = OBJECTS_DIR + "/" + hash.substr(0,2);
    const string path = dir + "/" + hash.substr(2);

    if(!fileExists(path)) createDir(dir);
    writeFile(path,content);

    return hash;
}

string IOManager::readBlob(const string &hash) {
    return readFile(OBJECTS_DIR + "/" + hash.substr(0,2) +"/"+hash.substr(2));
}

// Commit Metadata

bool IOManager::writeCommit(const string &hash, const string &data) {
    return writeFile(COMMITS_DIR + "/" + hash, data);
}

string IOManager::readCommit(const string &hash) {
    return readFile(COMMITS_DIR + "/" + hash);
}

// Reference Management

bool IOManager::writeReference(const string &refName, const string &hash) {
    if (refName == "HEAD") {
        return writeFile(HEAD_FILE, hash);
    }
    return writeFile(REFS_HEADS_DIR + "/" + refName, hash);
}

string IOManager::readReference(const string &refName) {
    if (refName == "HEAD") {
        string headRef = readFile(HEAD_FILE);
        if (headRef.find("ref: ") == 0) {
            // Trim the symbolic reference to remove any extra whitespace or newlines.
            string branchPath = utils::trim(headRef.substr(5));
            // Use MINIGIT_DIR constant so the path becomes ".minigit/refs/heads/main"
            return readFile(MINIGIT_DIR + "/" + branchPath);
        }
        return headRef;  // Detached HEAD mode
    }
    return readFile(REFS_HEADS_DIR + "/" + refName);
}

std::string IOManager::resolveHEAD() {
    std::string headContent = readFile(HEAD_FILE);
    if (headContent.rfind("ref:", 0) == 0) {
        std::string refPath = utils::trim(headContent.substr(5)); // trim any extra whitespace/newlines
        return readFile(MINIGIT_DIR + "/" + refPath);
    }
    return headContent; // detached mode
}

// Staging Area (Index)

bool IOManager::updateIndex(const vector<pair<string, string>> &entries) {
    ostringstream content;
    for (const auto &[filename, hash] : entries) {
        content << filename << ":" << hash << "\n";
    }
    return writeFile(INDEX_FILE, content.str());
}


vector<pair<string, string>> IOManager::readIndex() {
    vector<pair<string, string>> entries;
    
    if (!fileExists(INDEX_FILE)) {
        return entries;  // Empty staging area
    }

    try {
        string indexContent = readFile(INDEX_FILE);
        istringstream iss(indexContent);
        string line;
        
        while (getline(iss, line)) {
            size_t pos = line.find(':');
            if (pos != string::npos) {
                entries.emplace_back(
                    line.substr(0, pos),
                    line.substr(pos + 1)
                );
            }
        }
    } catch (...) {
        utils::displayError("Error reading index file.");
        return entries;
    }
    
    return entries;
}
