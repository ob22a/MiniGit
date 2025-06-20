#pragma once

#include <string>
#include <vector>

class CLI {
public:
    // Entry point: starts the interactive CLI loop
    static void run();

private:
    // Splits a user command string into command and arguments
    static std::vector<std::string> tokenize(const std::string& input);

    // Handles a parsed command (calls appropriate VCS function)
    static void executeCommand(const std::vector<std::string>& tokens);

    // Shows a help message listing available commands
    static void showHelp();
};

