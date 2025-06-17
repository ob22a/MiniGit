#include "io.hpp"
#include <fstream>
#include <sstream>
#include <openssl/sha.h>
#include <iostream>
#include <ctime>

using namespace std;
namespace fs = filesystem;

// Directory Operations

bool IOManager::initMinigitDir() {
    try {
        // Create repository structure
        create_directory(".minigit");
        create_directory(".minigit/objects");
        create_directory(".minigit/refs");
        create_directory(".minigit/refs/heads");

        // Initialize key files
        writeFile(".minigit/HEAD", "ref: refs/heads/main");
        writeFile(".minigit/index", "");

        return true;
    } catch (const exception &e) {
        cerr << "Error initializing repository: " << e.what() << endl;
        return false;
    }
}

bool IOManager::createDir(const string &path) {
    return create_directories(path);
}

// File Operations

string IOManager::readFile(const string &path) {
    ifstream file(path, ios::binary);
    if (!file) {
        throw runtime_error("Cannot open file: " + path);
    }
    
    return string(istreambuf_iterator<char>(file), 
                 istreambuf_iterator<char>());
}

bool IOManager::writeFile(const string &path, const string &content) {
    ofstream file(path, ios::binary);
    if (!file) {
        cerr << "Error writing to: " << path << endl;
        return false;
    }
    
    file << content;
    return file.good();
}

bool IOManager::copyFile(const string &src, const string &dest) {
    try {
        copy_file(src, dest, fs::copy_options::overwrite_existing);
        return true;
    } catch (const exception &e) {
        cerr << "Copy failed: " << e.what() << endl;
        return false;
    }
}

bool IOManager::fileExists(const string &path) {
    return exists(path);
}


// Blob Storage
string IOManager::writeBlob(const string &content) {
    const string hash = computeSHA1(content);
    const string path = ".minigit/objects/" + hash;

    if (!fileExists(path) && !writeFile(path, content)) {
        throw runtime_error("Failed to write blob: " + hash);
    }
    return hash;
}

string IOManager::readBlob(const string &hash) {
    return readFile(".minigit/objects/" + hash);
}

// Commit Metadata

bool IOManager::writeCommit(const string &hash, const string &data) {
    return writeFile(".minigit/objects/" + hash, data);
}

string IOManager::readCommit(const string &hash) {
    return readBlob(hash);
}

// Reference Management

bool IOManager::writeReference(const string &refName, const string &hash) {
    if (refName == "HEAD") {
        return writeFile(".minigit/HEAD", hash);
    }
    return writeFile(".minigit/refs/heads/" + refName, hash);
}

string IOManager::readReference(const string &refName) {
    if (refName == "HEAD") {
        string headRef = readFile(".minigit/HEAD");
        if (headRef.find("ref: ") == 0) {
            // Resolve symbolic reference
            string branchPath = headRef.substr(5);
            return readFile(".minigit/" + branchPath);
        }
        return headRef;  // Detached HEAD mode
    }
    return readFile(".minigit/refs/heads/" + refName);
}

// Staging Area (Index)

bool IOManager::updateIndex(const vector<pair<string, string>> &entries) {
    ostringstream content;
    for (const auto &[filename, hash] : entries) {
        content << filename << ":" << hash << "\n";
    }
    return writeFile(".minigit/index", content.str());
}

vector<pair<string, string>> IOManager::readIndex() {
    vector<pair<string, string>> entries;
    
    if (!fileExists(".minigit/index")) {
        return entries;  // Empty staging area
    }

    try {
        string indexContent = readFile(".minigit/index");
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
        // Return empty index on error
    }
    
    return entries;
}
