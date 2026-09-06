# include "CommandHandler.hpp"
# include "Server.hpp"
# include "Client.hpp"
# include "Parser.hpp"
# include "Channel.hpp"
# include "Replies.hpp"
# include <cctype>//isalpha,isalnum
# include <cstdlib>//atol

CommandHandler::CommandHandler(Server& srv) : server(srv){}
CommandHandler::~CommandHandler(){}

void CommandHandler::dispatchCommand(Client& client, const std::string& line)
{
    ParsedCommand parsed = Parser::parse(line);
    if (parsed.command.empty())
        return;
    const std::string& command = parsed.command;
    const std::vector<std::string>& params = parsed.params;
	if (!client.isRegistered() && command != "PASS" && command != "NICK"&& command != "USER"
        && command != "QUIT" && command != "PING")
    {
        client.appendOutput(Replies::notRegistered(client.getNickname()));
        return;
    }
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

/*ping:client send a ping to check the server is response
and the server answer with pong */
void CommandHandler::handlePing(Client& client, const std::vector<std::string>& param)
{
    if (param.empty())
    {
        client.appendOutput(Replies::noOrigin(client.getNickname()));
        return;
    }
    client.appendOutput(":ircserv PONG ircserv :" + param[0] + "\r\n");
}

/*pass:client provide the pass during registration and sercer check if it os right or no*/
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

/* nick : client choose a nickname ,the server validate ir and check
nobody have it also*/
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
        if (!std::isalnum(c) && c != '-' && c != '_')//no use of @#.! only nbr ,- _
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
	std::string oldNick = client.getNickname();

    client.setNickname(newNick);
    client.setNicknameSet(true);

    if (!wasRegistered && client.isRegistered())//why this because if an exist client change the nick dont send it again the messages
    {
        client.appendOutput(Replies::welcome(client.getNickname()));
        client.appendOutput(Replies::yourHost(client.getNickname()));
        client.appendOutput(Replies::created(client.getNickname()));
        client.appendOutput(Replies::myInfo(client.getNickname()));
    }
	else if (wasRegistered)
    {
        std::string message = ":" + oldNick+ " NICK :" + newNick + "\r\n";
        client.appendOutput(message);
        const std::vector<Channel*>& channels = server.getChannels();
        for (size_t i = 0; i < channels.size(); ++i)
        {
            if (channels[i]->isMember(&client))
                channels[i]->broadcast(message, &client);
        }
    }
}
/*user:client provide ir username real name during registration 
and server store them and complete the register*/
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

/*quit: client ask to disconnect the server ,we remove it from the server and the channels*/
void CommandHandler::handleQuit(Client& client, const std::vector<std::string>& param)
{
    std::string reason = "Quit";
    if (!param.empty())
        reason = param[0];
    std::string message = ":" + client.getNickname()+ " QUIT :" + reason + "\r\n";
    const std::vector<Channel*>& channels = server.getChannels();
    for (size_t i = 0; i < channels.size(); ++i)
    {
        if (channels[i]->isMember(&client))
            channels[i]->broadcast(message, &client);
    }
    server.removeClient(client);
}
/*
join:client ask to join a channel if the channel not exist we create it
otherwise we check if the client can join or no if he can wwe check why else we add it
*/
void CommandHandler::handleJoin(Client& client, const std::vector<std::string>& param)
{
    if (param.empty())
    {
        client.appendOutput(Replies::needMoreParams(client.getNickname(), "JOIN"));
        return;
    }
    const std::string& channelName = param[0];
    Channel* channel = server.findChannel(channelName);

    if (channel == NULL)
    {
        //channel doesn't exist yet ,create it, first joiner becomes operator
        channel = server.createChannel(channelName);
        channel->addMember(&client);
        channel->addOperator(&client);
		channel->removeInvite(&client);
    }
    else
    {
        if (channel->isMember(&client))
            return; //already in channel nothing to do
        std::string key;
        if (param.size() > 1)
            key = param[1];
        if (!channel->canJoin(&client, key))
        {
            if (channel->getInviteOnly() && !channel->isInvited(&client))
                client.appendOutput(Replies::inviteOnlyChannel(client.getNickname(), channelName));
            else if (channel->getHasKey() && key != channel->getKey())
                client.appendOutput(Replies::badChannelKey(client.getNickname(), channelName));
            else if (channel->channelFull())
                client.appendOutput(Replies::channelIsFull(client.getNickname(), channelName));
            return;
        }
        channel->addMember(&client);
		channel->removeInvite(&client);
    }
    channel->broadcast(":" + client.getNickname() + " JOIN " + channelName + "\r\n", &client);
    client.appendOutput(":" + client.getNickname() + " JOIN " + channelName + "\r\n");

    if (channel->getTopic().empty())
        client.appendOutput(Replies::noTopic(client.getNickname(), channelName));
    else
        client.appendOutput(Replies::topic(client.getNickname(), channelName, channel->getTopic()));

    std::string names;
    const std::vector<Client*>& members = channel->getMembers();
    for (size_t i = 0; i < members.size(); ++i)
    {
        if (i > 0)
            names += " ";
        if (channel->isOperator(members[i]))
            names += "@";
        names += members[i]->getNickname();
    }
    client.appendOutput(Replies::namesReply(client.getNickname(), channelName, names));
    client.appendOutput(Replies::endOfNames(client.getNickname(), channelName));
}

/*kick:channel operator ask to remove another client fro the channel
we check the permission and member notify the chanel then remove it
*/
void CommandHandler::handleKick(Client& client, const std::vector<std::string>& param)
{
    if (param.size() < 2)
    {
        client.appendOutput(Replies::needMoreParams(client.getNickname(), "KICK"));
        return;
    }
    const std::string& channelName = param[0];
    const std::string& nickname = param[1];
    Channel* channel = server.findChannel(channelName);
    if (channel == NULL)
    {
        client.appendOutput(Replies::noSuchChannel(client.getNickname(), channelName));
        return;
    }
    if (!channel->isMember(&client))
    {
        client.appendOutput(Replies::notOnChannel(client.getNickname(), channelName));
        return;
    }
    if (!channel->isOperator(&client))
    {
        client.appendOutput(Replies::chanOpPrivilegesNeeded(client.getNickname(), channelName));
        return;
    }

    Client* target = server.findClientByNickname(nickname);
    if (target == NULL)
    {
        client.appendOutput(Replies::noSuchNick(client.getNickname(), nickname));
        return;
    }
    if (!channel->isMember(target))
    {
        client.appendOutput(Replies::userNotInChannel(client.getNickname(),nickname,channelName));
        return;
    }
    std::string message =":" + client.getNickname() +" KICK " + channelName +" " + nickname + "\r\n";
    channel->broadcast(message, &client);
    client.appendOutput(message);
    channel->kick(target);
	if (channel->isEmpty())
    	server.removeChannel(channel);
}

/*topic: client ask to view or change the topic,we check that chennel exist,
and the client is an operator to know if he allow to access or modify it
*/
void CommandHandler::handleTopic(Client& client, const std::vector<std::string>& param)
{
    if (param.empty())
    {
        client.appendOutput(Replies::needMoreParams(client.getNickname(), "TOPIC"));
        return;
    }
    const std::string& channelName = param[0];
    Channel* channel = server.findChannel(channelName);
    if (channel == NULL)
    {
        client.appendOutput(Replies::noSuchChannel(client.getNickname(), channelName));
        return;
    }
    if (!channel->isMember(&client))
    {
        client.appendOutput(Replies::notOnChannel(client.getNickname(), channelName));
        return;
	}
    if (param.size() == 1)//topic #42 when we write this we ask for the current topic
    {
        if (channel->getTopic().empty())
        {
            client.appendOutput(Replies::noTopic(client.getNickname(), channelName));
        }
        else
        {
            client.appendOutput(Replies::topic(client.getNickname(),channelName,channel->getTopic()));
        }
        return;
    }
    //+t only channel operators can change the topic
    if (channel->getTopicRestricted() &&!channel->isOperator(&client))
    {
        client.appendOutput(Replies::chanOpPrivilegesNeeded(client.getNickname(), channelName));
        return;
    }
    const std::string& newTopic = param[1];//change the topic
    channel->setTopic(newTopic);
    std::string message =":" + client.getNickname() +" TOPIC " + channelName +" :" + newTopic + "\r\n";
    channel->broadcast(message, &client);
    client.appendOutput(message);
}

/*part : client ask to leave the channel ,we check uf the channel exist ,and the client
is member then remove the client from channel nottt server
*/
void CommandHandler::handlePart(Client& client, const std::vector<std::string>& param)
{
    if (param.empty())
    {
        client.appendOutput(Replies::needMoreParams(client.getNickname(), "PART"));
        return;
    }
    const std::string& channelName = param[0];
    Channel* channel = server.findChannel(channelName);

    if (channel == NULL)
    {
        client.appendOutput(Replies::noSuchChannel(client.getNickname(), channelName));
        return;
    }

    if (!channel->isMember(&client))
    {
        client.appendOutput(Replies::notOnChannel(client.getNickname(), channelName));
        return;
    }
    std::string message =":" + client.getNickname() +" PART " + channelName + "\r\n";
    channel->broadcast(message, &client);//tell other member
    client.appendOutput(message);
    channel->removeMember(&client);//remove the client from the channel
	channel->removeOperator(&client);
	if (channel->isEmpty())
        server.removeChannel(channel);
}

//part :the client can remove him self(client leave a channel ,but still connect to server and can join again)
//kick :the person who have the operatore remove
//quit:the client leave chanel and not connected to server

//-------nour

void CommandHandler::handlePrivmsg(Client& client, const std::vector<std::string>& param)
{
    if (param.empty())
    {    client.appendOutput(Replies::noRecipient(client.getNickname(), "PRIVMSG"));
        return;
    }
    else if (param.size() < 2)
    {
        client.appendOutput(Replies::noTextToSend(client.getNickname()));
        return ;
    }
    const std::string& target = param[0];
    const std::string& message = param[1];

    if (target[0] == '#')
    {
        Channel *a = server.findChannel(target);
        if (a == NULL)
        {
            client.appendOutput(Replies::noSuchChannel(client.getNickname(), target));
            return ;
        }
        if (!a->isMember(&client))
        {
            client.appendOutput(Replies::cannotSendToChannel(client.getNickname(), target));
            return ;
        }
        std::string line = ":" + client.getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        a->broadcast(line, &client);
    }
    else
    {
        Client* b = server.findClientByNickname(target);
        if (b == NULL)
        {
            client.appendOutput(Replies::noSuchNick(client.getNickname(), target));
            return ;
        }
        std::string line = ":" + client.getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        b->appendOutput(line);
    }
}
void CommandHandler::handleNotice(Client& client, const std::vector<std::string>& param)
{
    if (param.empty())
    {   return ;
    }
    else if (param.size() < 2)
    {
        return ;
    }
    const std::string& target = param[0];
    const std::string& message = param[1];

    if (target[0] == '#')
    {
        Channel *a = server.findChannel(target);
        if (a == NULL)
        {
            return ;
        }
        if (!a->isMember(&client))
        {
            return ;
        }
        std::string line = ":" + client.getNickname() + " NOTICE " + target + " :" + message + "\r\n";
        a->broadcast(line, &client);
    }
    else
    {
        Client* b = server.findClientByNickname(target);
        if (b == NULL)
        {
            return ;
        }
        std::string line = ":" + client.getNickname() + " NOTICE " + target + " :" + message + "\r\n";
        b->appendOutput(line);
    }
}

void CommandHandler::handleInvite(Client& client, const std::vector<std::string>& param)
{
    if (param.empty() || param.size() < 2)
        return ;

    const std::string& nickname = param[0];
    const std::string& channel = param[1];
    if (nickname == "" || channel == "")
    {
        client.appendOutput(Replies::needMoreParams(client.getNickname(), "INVITE"));
        return;
    }
    Channel *a = server.findChannel(channel);
    if (a == NULL)
    {
        client.appendOutput(Replies::noSuchChannel(client.getNickname(), channel));
        return;
    }
    if (!a->isMember(&client))
    {
        client.appendOutput(Replies::notOnChannel(client.getNickname(), channel));
        return;
    }
    if (a->getInviteOnly())
    {
        if (!a->isOperator(&client))
        {
            client.appendOutput(Replies::chanOpPrivilegesNeeded(client.getNickname(), channel));
            return;
        }
    }
    Client *target = server.findClientByNickname(nickname);
    if ( target == NULL)
    {
        client.appendOutput(Replies::noSuchNick(client.getNickname(), nickname));
        return;
    }
    if (a->isMember(target))
    {
        client.appendOutput(Replies::userOnChannel(client.getNickname(), nickname, channel));
        return;
    }
    a->addInvite(target);
    client.appendOutput(Replies::inviting(client.getNickname(), nickname, channel));
        std::string line = ":" + client.getNickname() + " INVITE " + nickname + " :" + channel + "\r\n";
    target->appendOutput(line);
}

void CommandHandler::handleMode(Client& client, const std::vector<std::string>& param)
{
    if (param.empty() || param.size() < 2)
    {
        client.appendOutput(Replies::needMoreParams(client.getNickname(), "MODE"));
        return;
    }

    const std::string& channel = param[0];
    Channel *a = server.findChannel(channel);
    if (a == NULL)
    {
        client.appendOutput(Replies::noSuchChannel(client.getNickname(), channel));
        return;
    }
    if (!a->isOperator(&client))
    {
        client.appendOutput(Replies::chanOpPrivilegesNeeded(client.getNickname(), channel));
        return;
    }

    std::string mode = param[1];
    int paramsize = (int)param.size();
    char sign = '\0'; //instead of null
    int parameterspointer = 1;

    for (size_t i = 0; i < mode.size(); i++)
    {
        if (mode[i] == '+' || mode[i] == '-')
        {
            sign = mode[i];
        }
        else if (mode[i] == 'k' || mode[i] == 'o' || mode[i] == 'l')
        {
            if (sign == '\0')
            {
                client.appendOutput(Replies::unknownMode(client.getNickname(), std::string(1, mode[i])));
                return;
            }
            if (sign == '+')
            {
                if (mode[i] == 'k')
                {
                    parameterspointer++;
                    if (parameterspointer >= paramsize)
                    {
                        client.appendOutput(Replies::needMoreParams(client.getNickname(), "MODE"));
                        return;
                    }
                    a->setKey(param[parameterspointer]);
                }
                else if (mode[i] == 'o')
                {
                    parameterspointer++;
                    if (parameterspointer >= paramsize)
                    {
                        client.appendOutput(Replies::needMoreParams(client.getNickname(), "MODE"));
                        return;
                    }
                    const std::string& targetNick = param[parameterspointer];
                    Client *cl = server.findClientByNickname(targetNick);
                    if (cl == NULL)
                    {
                        client.appendOutput(Replies::noSuchNick(client.getNickname(), targetNick));
                        return;
                    }
                    if (!a->isMember(cl))
                    {
                        client.appendOutput(Replies::userNotInChannel(client.getNickname(), targetNick, channel));
                        return;
                    }
                    a->addOperator(cl);
                }
                else if (mode[i] == 'l')
                {
                    parameterspointer++;
                    if (parameterspointer >= paramsize)
                    {
                        client.appendOutput(Replies::needMoreParams(client.getNickname(), "MODE"));
                        return;
                    }
                    long value = std::atol(param[parameterspointer].c_str());
                    if (value < 0)
                        value = 0;
                    a->setUserLimit((size_t)value);
                }
            }
            else //sign == '-'
            {
                if (mode[i] == 'k')
                {
                    a->removeKey();
                }
                else if (mode[i] == 'o')
                {
                    parameterspointer++;
                    if (parameterspointer >= paramsize)
                    {
                        client.appendOutput(Replies::needMoreParams(client.getNickname(), "MODE"));
                        return;
                    }
                    const std::string& targetNick = param[parameterspointer];
                    Client *cl = server.findClientByNickname(targetNick);
                    if (cl == NULL)
                    {
                        client.appendOutput(Replies::noSuchNick(client.getNickname(), targetNick));
                        return;
                    }
                    if (!a->isMember(cl))
                    {
                        client.appendOutput(Replies::userNotInChannel(client.getNickname(), targetNick, channel));
                        return;
                    }
                    a->removeOperator(cl);
                }
                else if (mode[i] == 'l')
                {
                    a->removeUserLimit();
                }
            }
        }
        else if (mode[i] == 'i' || mode[i] == 't')
        {
            bool flag = false;
            if (sign == '+')
                flag = true;
            else if (sign == '-')
                flag = false;
            else
            {
                client.appendOutput(Replies::unknownMode(client.getNickname(), std::string(1, mode[i])));
                return;
            }
            if (mode[i] == 'i')
                a->setInviteOnly(flag);
            else
                a->setTopicRestricted(flag);
        }
        else
        {
            client.appendOutput(Replies::unknownMode(client.getNickname(), std::string(1, mode[i])));
            return;
        }
    }
    std::string line = ":" + client.getNickname() + " MODE " + channel;
    for (int j = 1; j <= parameterspointer; ++j)
        line += " " + param[j];
    line += "\r\n";
    a->broadcast(line, &client);
    client.appendOutput(line);
}