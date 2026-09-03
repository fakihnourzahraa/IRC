
#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

struct ParsedCommand
{
    std::string command;//the cmnd
    std::vector<std::string> params;//the array[]
};

class Parser
{
public:
    static ParsedCommand parse(const std::string& line);
};

#endif