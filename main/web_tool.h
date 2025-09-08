#pragma once

#include <string>

// This is the function that will do the actual work.
std::string perform_web_search(const std::string& query);

// This function will register our tool with the MCP server.
void register_web_tool();