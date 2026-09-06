# include "Server.hpp"
# include <iostream>
# include <cstdlib>
# include <cctype>
# include <string>

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Should be " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    std::string portStr = argv[1];
    if (portStr.empty())
    {
        std::cerr << "Error: port cannot be empty" << std::endl;
        return 1;
    }
    for (size_t i = 0; i < portStr.size(); ++i)
    {
        if (!std::isdigit((unsigned char)portStr[i]))
        {
            std::cerr << "Error: port must be a number" << std::endl;
            return 1;
        }
    }
    long port = std::atol(portStr.c_str());
    if (port < 1024 || port > 65535)
    {
        std::cerr << "Error: port must be between 1024 and 65535" << std::endl;
        return 1;
    }
    std::string password = argv[2];
    if (password.empty())
    {
        std::cerr << "Error: password cannot be empty" << std::endl;
        return 1;
    }
    try
    {
        Server server((int)port, password);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}