# MiniGit System

MiniGit is a simplified, local-only version control system that mimics the core features of Git. It models commit history as a Directed Acyclic Graph (DAG), tracks changes using simulated SHA-1 hashing, and provides basic command-line interface functionality.

---

## 🧱 Data Structures Used

1. **Directed Acyclic Graph (DAG)**
   - Represents commit history
   - Nodes are commits pointing to their parent(s)
   - Enables history traversal, merging, and finding the least common ancestor (LCA)

2. **Simulated SHA-1 Hashing**
   - We use `std::hash<std::string>` or a simplified hash function instead of the actual SHA-1 algorithm
   - Provides content-based identifiers for blobs and commits
   - Not cryptographically secure — suitable for educational/testing use only

3. **File System Hierarchy (`.minigit/`)**
   - `objects/`: Stores file contents ("blobs") named by their hash
   - `commits/`: Contains metadata for each commit
   - `refs/heads/`: Tracks branch tips
   - `index`: The staging area that maps filenames to hashes

4. **Index (Staging Area)**
   - A simple key-value store mapping file names to content hashes
   - Used to prepare changes before committing

5. **HEAD Pointer**
   - A text file that points to the current branch or commit (in detached mode)

6. **Conflict Marker Blocks**
   - Used during merges to show unresolved conflicts
   - Follows Git-style formatting: `<<<<<<<`, `=======`, `>>>>>>>`

---

## 🧠 Design Decisions

- **Simulated SHA-1 for Content Addressing**: Balances conceptual accuracy with simplicity
- **Modular Code Structure**: Each team member is responsible for a module (e.g., `vcs.cpp`, `io.cpp`)
- **Header-Only Interfaces**: Modules communicate via public headers to promote encapsulation
- **CLI-Oriented Workflow**: All commands are parsed and executed via `cli.cpp`, similar to Git
- **Conflict Formatting**: Mirrors Git’s conflict markers to make manual resolution intuitive
- **Error Handling**: Consistent return codes and messages used across all operations

---

## ⚠️ Limitations

- **No Remote Repositories**: MiniGit operates only locally
- **Basic Merge Support**: Only three-way merge with minimal conflict resolution
- **No Rename/Deletion Tracking**: Index only supports additions and modifications
- **Insecure Hashing**: No cryptographic guarantees or collision resistance
- **No Advanced Features**: Missing stash, reflog, blame, cherry-pick, etc.
- **Inefficient Storage**: Full content is stored for each version (no delta compression)
- **Plain CLI**: No colors, autocomplete, or interactive help

---

## 🚀 Future Enhancements

- Remote repository support (push/pull/sync)
- Graphical or web-based commit visualization
- Smarter merge resolution and diff tools
- File renames, deletions, permission tracking
- Compression and garbage collection for objects
- Unit testing and CI/CD hooks
- Custom user commands and plug-in framework

---

## 💻 Compilation & Usage

To build the project, compile all source files using `g++`:

```bash
g++ src/cli.cpp src/dsa.cpp src/io.cpp src/utils.cpp src/vcs.cpp main.cpp -o minigit
````

Then run the resulting executable:

```bash
./minigit          # On Linux/macOS
minigit.exe        # On Windows (after building)
```

## 📂 Repository Structure

```
minigit/
├── src/
│   ├── cli.cpp / cli.hpp     # Command-line interface logic
│   ├── vcs.cpp / vcs.hpp     # Core version control engine
│   ├── dsa.cpp / dsa.hpp     # Data structures and hashing
│   ├── io.cpp / io.hpp       # File operations and repository structure
│   ├── utils.cpp / utils.hpp # Utility functions (trimming, error handling, etc.)
├── main.cpp                  # Entry point
├── README.md
```

### 🏗️ VCS Internal Structure (created by `init`)

```
.minigit/
├── objects/           # Stores file contents (blobs) named by hash
├── commits/           # Stores commit metadata
├── refs/
│   └── heads/         # Branch references (e.g., main, feature-x)
├── HEAD               # Points to current branch or commit
├── index              # Staging area for tracked files
```

---

## 📌 Notes on Simulated Hashing

For simplicity and performance, the project uses a built-in non-cryptographic hash function (`std::hash<std::string>`) instead of a full SHA-1 implementation. This allows us to:

* Simulate content-addressing (like Git's object model)
* Deduplicate and identify blobs/commits
* Avoid unnecessary complexity in an educational project

However, this means hashes:

* Are not secure or collision-resistant
* Should not be used in real-world distributed systems
