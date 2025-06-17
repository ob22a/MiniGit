#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <sstream>
#include <openssl/sha.h>

using namespace std;
namespace fs = std::filesystem;

class IOManager
{
public:
    // === Directory Setup ===
    // Creates the .minigit directory structure
    static bool initMinigitDir();
    // Creates a directory at the given path (including parents)
    static bool createDir(const string &path);

    // === File Operations ===
    // Reads the entire contents of a file into a string
    static string readFile(const string &path);
    // Writes the given content to a file, overwriting if it exists
    static bool writeFile(const string &path, const string &content);
    // Copies a file from src to dest, returns success
    static bool copyFile(const string &src, const string &dest);
    // Checks if a file or directory exists at the path
    static bool fileExists(const string &path);

    // === Blob Storage ===
    // Writes content as a blob in objects/ and returns its SHA-1 hash
    static string writeBlob(const string &content);
    // Reads a blob's content by its hash
    static string readBlob(const string &hash);

    // === Commit Metadata ===
    // Writes commit data (parent, tree, timestamp, message) under objects/
    static bool writeCommit(const string &hash, const string &data);
    // Reads commit data by its hash
    static string readCommit(const string &hash);

  
    // Writes a branch or HEAD reference to refs/heads/
    static bool writeReference(const string &refName, const string &hash);
    // Reads a branch or HEAD reference from refs/heads/
    static string readReference(const string &refName);

    // Staging Area (Index)
    // Saves the staging entries (filename: hash pairs) to .minigit/index
    static bool updateIndex(const vector<pair<string, string>> &entries);
    // Loads staging entries from .minigit/index into a vector of pairs
    static vector<pair<string, string>> readIndex();
};

