#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP
#include <string>
#include <vector>

class Client;
class Server;

class CommandHandler
{
private:
    Server& server;//reference back to the server that owns us

public:
    CommandHandler(Server& server);
    ~CommandHandler();
    void dispatchCommand(Client& client, const std::string& line);
    //IRC commands
    void handlePing(Client& client, const std::vector<std::string>& param);
    void handlePass(Client& client, const std::vector<std::string>& param);
    void handleNick(Client& client, const std::vector<std::string>& param);
    void handleUser(Client& client, const std::vector<std::string>& param);
    void handleQuit(Client& client, const std::vector<std::string>& param);
    void handleJoin(Client& client, const std::vector<std::string>& param);
    void handlePart(Client& client, const std::vector<std::string>& param);
	void handleTopic(Client& client, const std::vector<std::string>& param);
	void handleKick(Client& client, const std::vector<std::string>& param);
    void handlePrivmsg(Client& client, const std::vector<std::string>& param);
    void handleNotice(Client& client, const std::vector<std::string>& param);
    void handleInvite(Client& client, const std::vector<std::string>& param);
    void handleMode(Client& client, const std::vector<std::string>& param);
};

#endif