#include "cli.hpp"
#include "vcs.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

void CLI::run() {
    std::string input;
    std::cout << " ----- Welcome to MiniGit! Type 'help' for commands. -----\n\n";

    while (true) {
        std::cout << "minigit> ";
        std::getline(std::cin, input);
        if (input.empty()) continue;

        std::vector<std::string> tokens = tokenize(input);
        if (tokens.empty()) continue;

        std::string command = tokens[0];
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);

        if (command == "exit") {
            std::cout << "Exiting MiniGit... \n";
            break;
        }

        executeCommand(tokens);
    }
}

std::vector<std::string> CLI::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;

    while (iss >> std::quoted(token)) {
        tokens.push_back(token);
    }

    return tokens;
}

void CLI::executeCommand(const std::vector<std::string>& tokens) {
    using namespace vcs;

    const std::string& cmd = tokens[0];

    try {
        if (cmd == "help") showHelp();
        else if (cmd=="cls" || cmd=="clear"){
            system("clear");
        } 
        else if (cmd == "init") {
            init();
        } 
        else if (cmd == "add") {
            if (tokens.size() != 2) std::cout << "Usage: add <filename>\n";
            else  add(tokens[1]);

        } 
        else if (cmd == "commit") {
            if (tokens.size() >= 3 && tokens[1] == "-m") {
                std::string msg;
                for (size_t i = 2; i < tokens.size(); ++i) {
                    if (i > 2) msg += " ";
                    msg += tokens[i];
                }
                commit(msg);
            } 
            else std::cout << "Usage: commit -m \"message\"\n";

        } 
        else if (cmd == "log") {
            log();
        } 
        else if (cmd == "branch") {
            if (tokens.size() != 2) std::cout << "Usage: branch <name>\n";
            else branch(tokens[1]);
        } 
        else if (cmd == "checkout") {
            if (tokens.size() != 2) std::cout << "Usage: checkout <branch|hash>\n"; 
            else checkout(tokens[1]);
        } 
        else if (cmd == "merge") {
            if (tokens.size() != 2) std::cout << "Usage: merge <branch>\n";
            else  merge(tokens[1]);

        } 
        else if (cmd == "diff") {
            if (tokens.size() != 3) std::cout << "Usage: diff <commit1> <commit2>\n";
            else diff(tokens[1], tokens[2]);

        } 
        else std::cout << " Unknown or malformed command. Type 'help'.\n";

    } catch (const std::exception& ex) {
        std::cerr << " Error: " << ex.what() << std::endl;
    }
}

void CLI::showHelp() {
    std::cout << R"(Available commands:
  init                   Initialize a new MiniGit repo
  add <file>             Stage a file
  commit -m "<msg>"      Commit staged files with message
  log                    Show commit history
  branch <name>          Create a new branch
  checkout <name|hash>   Switch to branch or commit
  merge <branch>         Merge another branch
  diff <c1> <c2>         Show diff between two commits
  cls/clear              Clear the screen
  help                   Show this message
  exit                   Quit MiniGit
)" << std::endl;
}
