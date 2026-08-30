#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Parser.hpp"
#include "Replies.hpp"
#include <cctype>//isalpha,isalnum

CommandHandler::CommandHandler(Server& srv) : server(srv){}
CommandHandler::~CommandHandler(){}

void CommandHandler::dispatchCommand(Client& client, const std::string& line)
{
    ParsedCommand parsed = Parser::parse(line);
    if (parsed.command.empty())
        return;
    const std::string& command = parsed.command;
    const std::vector<std::string>& params = parsed.params;

    if (command == "PASS")
        handlePass(client, params);
    else if (command == "NICK")
        handleNick(client, params);
    else if (command == "USER")
        handleUser(client, params);
    else if (command == "JOIN")
        handleJoin(client, params);
    else if (command == "PART")
        handlePart(client, params);
    else if (command == "PRIVMSG")
        handlePrivmsg(client, params);
    else if (command == "NOTICE")
        handleNotice(client, params);
    else if (command == "TOPIC")
        handleTopic(client, params);
    else if (command == "INVITE")
        handleInvite(client, params);
    else if (command == "KICK")
        handleKick(client, params);
    else if (command == "MODE")
        handleMode(client, params);
    else if (command == "QUIT")
        handleQuit(client, params);
    else if (command == "PING")
        handlePing(client, params);
    else
        client.appendOutput(Replies::unknownCommand(client.getNickname(), command));
}

void CommandHandler::handlePing(Client& client, const std::vector<std::string>& param)
{
    if (param.empty())
    {
        client.appendOutput(Replies::noOrigin(client.getNickname()));
        return;
    }
    client.appendOutput(":ircserv PONG ircserv :" + param[0] + "\r\n");
}

void CommandHandler::handlePass(Client& client,const std::vector<std::string>& param)
{
    if (client.isPasswordAccepted())
    {
        client.appendOutput(Replies::alreadyRegistered(client.getNickname()));//if he do twice pass xx
        return;
    }
    if (param.empty())
    {
        client.appendOutput(Replies::needMoreParams(client.getNickname(), "PASS"));//when do pass itshould be a paramt after it
        return;
    }
    if (param[0] != server.getPassword())
    {
        client.appendOutput(Replies::passwordMismatch(client.getNickname()));
        return;
    }
    client.setPasswordAccepted(true);//if write pass right put it true
}

void CommandHandler::handleNick(Client& client,const std::vector<std::string>& param)
{
    if (param.empty())
    {
        client.appendOutput(Replies::noNicknameGiven(client.getNickname()));
        return;
    }
    const std::string& newNick = param[0];
    if (newNick.empty() || !std::isalpha(newNick[0]))
    {
        client.appendOutput(Replies::erroneousNickname(client.getNickname(), newNick));
        return;
    }//check first char is alpha
    for (size_t i = 1; i < newNick.size(); ++i)//checkafter first character
    {
        char c = newNick[i];
        if (!std::isalnum(c) && c != '-' && c != '_')//no use of @#.!
        {
            client.appendOutput(Replies::erroneousNickname(client.getNickname(), newNick));
            return;
        }
    }
    Client* existing = server.findClientByNickname(newNick);//point to an address
    if (existing != NULL && existing != &client)//check if nickname in use existing !=null mean enu nickname in use
    {
		//so we search first if !=null we see if these clien & same as the one do nick
        client.appendOutput(Replies::nicknameInUse(client.getNickname(), newNick));
        return;
    }
	bool wasRegistered = client.isRegistered();
    client.setNickname(newNick);
    client.setNicknameSet(true);
    if (!wasRegistered && client.isRegistered())//why this because if an exist client change the nick dont send it again the messages
    {
        client.appendOutput(Replies::welcome(client.getNickname()));
        client.appendOutput(Replies::yourHost(client.getNickname()));
        client.appendOutput(Replies::created(client.getNickname()));
        client.appendOutput(Replies::myInfo(client.getNickname()));
    }
}

void CommandHandler::handleUser(Client& client,const std::vector<std::string>& param)
{
    if (client.isRegistered())
    {
        client.appendOutput(Replies::alreadyRegistered(client.getNickname()));
        return;
    }
    if (param.size() < 4)//bcz USER <username> <mode> <unused> :<realname>
    {
        client.appendOutput(Replies::needMoreParams(client.getNickname(), "USER"));
        return;
    }
    client.setUsername(param[0]);
    client.setRealname(param[3]);
    client.setUsernameSet(true);

    if (client.isRegistered())
    {
        client.appendOutput(Replies::welcome(client.getNickname()));
        client.appendOutput(Replies::yourHost(client.getNickname()));
        client.appendOutput(Replies::created(client.getNickname()));
        client.appendOutput(Replies::myInfo(client.getNickname()));
    }
}
// ----- not implemented yet to test
//----mira
void CommandHandler::handleQuit(Client& client, const std::vector<std::string>& param)
{
    (void)param;
    server.removeClient(client);//receiveData() checks fd existence after this, so this is safe
}
void CommandHandler::handleJoin(Client& client, const std::vector<std::string>& param)
{
    (void)param;
    client.appendOutput(Replies::unknownCommand(client.getNickname(), "JOIN"));
}
void CommandHandler::handleKick(Client& client, const std::vector<std::string>& param)
{
    (void)param;
    client.appendOutput(Replies::unknownCommand(client.getNickname(), "KICK"));
}
void CommandHandler::handleTopic(Client& client, const std::vector<std::string>& param)
{
    (void)param;
    client.appendOutput(Replies::unknownCommand(client.getNickname(), "TOPIC"));
}
void CommandHandler::handlePart(Client& client, const std::vector<std::string>& param)
{
    (void)param;
    client.appendOutput(Replies::unknownCommand(client.getNickname(), "PART"));
}

//-------nour
void CommandHandler::handlePrivmsg(Client& client, const std::vector<std::string>& param)
{
    (void)param;
    client.appendOutput(Replies::unknownCommand(client.getNickname(), "PRIVMSG"));
}
void CommandHandler::handleNotice(Client& client, const std::vector<std::string>& param)
{
    (void)client;
    (void)param;

}
void CommandHandler::handleInvite(Client& client, const std::vector<std::string>& param)
{
    (void)param;
    client.appendOutput(Replies::unknownCommand(client.getNickname(), "INVITE"));
}

void CommandHandler::handleMode(Client& client, const std::vector<std::string>& param)
{
    (void)param;
    client.appendOutput(Replies::unknownCommand(client.getNickname(), "MODE"));
}