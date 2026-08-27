#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <sys/socket.h>//socket,bind,listen...
#include <unistd.h>//close
#include <fcntl.h>//for fcntl
#include <iostream>//cout,cerr
#include <stdexcept>//exception
#include <cstring>//for memset
#include <sstream>//string stream
#include <cctype>//toupper...
#include <netinet/in.h>//sockaddrs_in

Server::Server(int portNumber, const std::string& serverPassword)
{
    port = portNumber;
    password = serverPassword;
    listenFd = -1;//mean no valid socket
}
Server::~Server()
{
	size_t i;
	for (i = 0; i < clients.size(); i++)
	{
		close(clients[i]->getFd());
		delete clients[i];
	}
	for (i = 0; i < channels.size(); i++)
		delete channels[i];
	if (listenFd != -1)
		close(listenFd);
}
void Server::setupSocket()
{//the flow:	//socket->setsockopt->bind->listen->fcntl
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
        throw std::runtime_error("socket() failed");
    int opt = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in address;//we need it for bind
    std::memset(&address, 0, sizeof(address));//initial to 0 to make sure we do right
    address.sin_family = AF_INET;//ipv4
    address.sin_addr.s_addr = INADDR_ANY;//any local host
    address.sin_port = htons(port);//transform from computer byte to network byte

    if (bind(listenFd, (struct sockaddr*)&address, sizeof(address)) < 0)
        throw std::runtime_error("bind() failed");

    if (listen(listenFd, 128) < 0)//128 pending request not 128 client
        throw std::runtime_error("listen() failed");

    fcntl(listenFd, F_SETFL, O_NONBLOCK);//nonblock make recv send return immediately instead of blocking
    struct pollfd pfd;//now we have the listenfd and server use poll() so pollfd have info
						//the poll should monitor
    pfd.fd = listenFd;
    pfd.events = POLLIN;//when new connection arrive wait for input
	//we dont use pollout bcz pollout more focus on outp
    pfd.revents = 0;//we initial by 0 then poll overwrite it
    pollFds.push_back(pfd);
}

void Server::acceptNewClient()
{
    struct sockaddr_in clientAddress;//create struc where accept will store info about client
    socklen_t clientLen = sizeof(clientAddress);//tell accept size of the clientAddr struc
    //listenFd=the serv listen socket/ clientAddr=where accept can store the client add/client len lengt of struct
    
    int clientFd = accept(listenFd,(struct sockaddr*)&clientAddress,&clientLen);
	//accept returns a newfile descriptor for the client
    if (clientFd < 0)
        return;
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(clientFd);
        return;//we close it and stop handle it
    }

    Client* client = new Client(clientFd);
    clients.push_back(client);//we push it to client vector

    struct pollfd pfd;//creat a pollfd for the new client 
	//poll() need pollfd for each client
    pfd.fd = clientFd;//tell poll that we work by the clientfd
    pfd.events = POLLIN;//to know when this client have data avail to read
    pfd.revents = 0;
    pollFds.push_back(pfd);
}

void Server::receiveData(Client& client)
{
    char buffer[4096];//temporary buff to reciev byte from client
    int bytesReceived = recv( client.getFd(),buffer,sizeof(buffer),0);

    if (bytesReceived == 0)//when the client close the conn
    {
        removeClient(client);//remove jt from server
        return;
    }
    if (bytesReceived < 0)//if recv failed
    {
        removeClient(client);
        return;
    }
    client.appendInput(std::string(buffer, bytesReceived));
    //check if inputbuff have at leat an irc cmnd end with\r\n
    while (client.hasCompleteLine())
    {
        std::string line = client.extractLine();//take one inoput out of inputbuff
        dispatchCommand(client, line);//identify the command 
    }
}

void Server::sendData(Client& client)
{
    //if there is nothing to sent,there is nothing for this fct to do
    if (!client.hasOutput())
        return;
    //this contains data that the server wantsto send to client
    const std::string& output = client.getOutput();
    // Send the data through the client's socket.
    int bytesSent = send(client.getFd(),output.c_str(),output.size(),0);
    if (bytesSent < 0)//if send failed.
    {
        removeClient(client);
        return;
    }
    //remove only the byte that we actually send,this is important
    //send might send only part of the output buffer.
    client.removeSentData(bytesSent);
}

void Server::removeClient(Client& client)
{
    int fd = client.getFd();//save the fc client we want to delete

    //we remove the client from all the channels we have
    for (std::vector<Channel*>::iterator it = channels.begin();it != channels.end();++it)
    {
        // Check if this client is a member of the current channel.
        if ((*it)->isMember(&client))
        {
            (*it)->removeMember(&client);//if is a member remove it from this channel
        }
    }
    //search through all file descriptors monitored by poll() to remove it from the vector
    for (std::vector<struct pollfd>::iterator it = pollFds.begin();it != pollFds.end();++it)
    {
        if (it->fd == fd)
        {
            pollFds.erase(it);
            break;//we find it no need to continue serach so break
        }
    }
    close(fd);
    //search for this Client object in the clients vector
    for (std::vector<Client*>::iterator it = clients.begin();it != clients.end();++it)
    {
        // Check if this pointer points to the Client we want to remove
        if (*it == &client)
        {
            delete *it;//bcz we created as new
            clients.erase(it);// Remove the pointer from the clients vector
            break;
        }
    }
}

Channel* Server::findChannel(const std::string& name)
{
    //go through all channels stored in the server
    for (std::vector<Channel*>::iterator it = channels.begin();it != channels.end();++it)
    {
        //check if the current chhanel is the ome we search for
        if ((*it)->getName() == name)
        {
            return *it;
        }
    }
    return NULL;//if no one retuen null
}
Client* Server::findClientByFd(int fd)
{
    //go through all clients connected to the server.
    for (std::vector<Client*>::iterator it = clients.begin();it != clients.end();++it)
    {
        //check if the current client is the one
        if ((*it)->getFd() == fd)
        {
            return *it;
        }
    }
    return NULL;
}
Client* Server::findClientByNickname(const std::string& nickname)
{
    for (std::vector<Client*>::iterator it = clients.begin();it != clients.end();++it)
    {
        if ((*it)->getNickname() == nickname)
            return *it;
    }
    return NULL;
}

void Server::dispatchCommand(Client& client, const std::string& line)
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

void Server::handlePing(Client& client, const std::vector<std::string>& param)
{
    if (param.empty())
    {
        client.appendOutput(Replies::noOrigin(client.getNickname()));
        return;
    }
    client.appendOutput(":ircserv PONG ircserv :" + param[0] + "\r\n");
}

void Server::handlePass(Client& client,const std::vector<std::string>& param)
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
    if (param[0] != password)
    {
        client.appendOutput(Replies::passwordMismatch(client.getNickname()));
        return;
    }
    client.setPasswordAccepted(true);//if write pass right put it true
}

void Server::handleNick(Client& client,const std::vector<std::string>& param)
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
    Client* existing = findClientByNickname(newNick);//point to an address
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

void Server::handleUser(Client& client,const std::vector<std::string>& param)
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