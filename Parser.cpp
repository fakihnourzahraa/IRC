#include "Parser.hpp"
#include <cctype>

ParsedCommand Parser::parse(const std::string& line)
{
    ParsedCommand result;
    size_t i = 0;
    //skip spaces before the command
    while (i < line.size() && std::isspace(line[i]))//check ' ' .\t,\n bool
        ++i;
    //empty line
    if (i == line.size())
        return result;
    //get command read the cmnd and save it into command
    while (i < line.size() && !std::isspace(line[i]))
    {
        result.command += line[i];
        ++i;
    }
    //after we have the cmnd we do it upper
    for (size_t j = 0; j < result.command.size(); ++j)
        result.command[j] = std::toupper(result.command[j]);

    //now we should know the parametrs for params[]
    while (i < line.size())
    {
        //skip spaces between cmnd and paramter if there is a space
        while (i < line.size() && std::isspace(line[i]))
            ++i;
        if (i == line.size())//only cmnd exist
            break;
        if (line[i] == ':')
        {
            ++i;//to become on the next charact after :
            result.params.push_back(line.substr(i));
            break;
        }
        //normal parameter
        std::string param;
        while (i < line.size() && !std::isspace(line[i]))
        {
            param += line[i];
            ++i;
        }
        result.params.push_back(param);
    }
    return result;
}