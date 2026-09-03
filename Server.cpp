#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"
#include <sys/socket.h>//socket,bind,listen...
#include <unistd.h>//close
#include <fcntl.h>//for fcntl
#include <iostream>//cout,cerr
#include <stdexcept>//exception
#include <cstring>//for memset
#include <sstream>//string stream
#include <cctype>//toupper...
#include <netinet/in.h>//sockaddrs_in
#include <csignal>

Server::Server(int portNumber, const std::string& serverPassword)
{
    port = portNumber;
    password = serverPassword;
    listenFd = -1;//mean no valid socket
    commandHandler = new CommandHandler(*this);
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
	delete commandHandler;
}
void Server::handleSignal(int signum)
{
    (void)signum;
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
	struct sigaction sa;
    sa.sa_handler = Server::handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

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
    int fd = client.getFd();
	int bytesReceived = recv( fd,buffer,sizeof(buffer),0);

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
    while (findClientByFd(fd) != NULL && client.hasCompleteLine())
    {
        std::string line = client.extractLine();//take one inoput out of inputbuff
        commandHandler->dispatchCommand(client, line);//identify the command 
    }
}

void Server::sendData(Client& client)
{
    //if there is nothing to sent,there is nothing for this fct to do
    if (!client.hasOutput())
        return;
    //this contains data that the server wantsto send to client
    const std::string& output = client.getOutputBuffer();
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
    for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); )
	{
		if ((*it)->isMember(&client))
		{
			(*it)->removeMember(&client);
			(*it)->removeOperator(&client);
			(*it)->removeInvite(&client);
		}
		if ((*it)->isEmpty())
		{
			delete *it;
			it = channels.erase(it);
		}
		else
			++it;
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
Channel* Server::createChannel(const std::string& name)
{
    Channel* channel = new Channel(name);
    channels.push_back(channel);
    return channel;
}
void Server::removeChannel(Channel* channel)
{
    for (std::vector<Channel*>::iterator it = channels.begin();it != channels.end(); ++it)
    {
        if (*it == channel)
        {
            delete *it;
            channels.erase(it);
            return;
        }
    }
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
const std::vector<Channel*>& Server::getChannels() const
{
    return channels;
}

const std::string& Server::getPassword() const
{
    return password;
}

void Server::run()
{
    setupSocket();//we create the socke and prepare it
    std::cout << "ircserv listening on port " << port << std::endl;

    while (true)//infinite loop to keep running
    {
        for (size_t i = 0; i < pollFds.size(); ++i)
        {
            if (pollFds[i].fd == listenFd)
                continue;//skip the listen fd bcz its only watch (pollin)
            Client* c = findClientByFd(pollFds[i].fd);
            if (c != NULL && c->hasOutput())
                pollFds[i].events = POLLIN | POLLOUT;//pollout when server has also data to send to client
            else
                pollFds[i].events = POLLIN;//when client send data
        }
        int ready = poll(&pollFds[0], pollFds.size(), -1);//main thing call the poll its wait until an event happen
        if (ready < 0)//if fail yhe poll
        {
            break;//real poll() failure, stop the server
        }

        for (size_t i = 0; i < pollFds.size(); ++i)
        {
            if (pollFds[i].revents == 0)
                continue;//ignore it when nothing happen

            size_t sizeBefore = pollFds.size();//bcz later we want to remobve client
            int fd = pollFds[i].fd;
            if (fd == listenFd)
            {
                if (pollFds[i].revents & POLLIN)
                    acceptNewClient();
            }
            else//not the listensocket so client(deal wigh an exist client)
            {
                Client* c = findClientByFd(fd);
                if (c == NULL)
                    continue;//already gone this round

                if (pollFds[i].revents & (POLLHUP | POLLERR))
                {
                    removeClient(*c);
                }//hangup or error occured we delete it
                else//client hasnt disconnect or produce error
                {
                    if (pollFds[i].revents & POLLIN)
                    {
                        receiveData(*c);
                        c = findClientByFd(fd);
                    }
                    if (c != NULL && (pollFds[i].revents & POLLOUT))
                        sendData(*c);
                }
            }
            if (pollFds.size() < sizeBefore)
                --i;
        }
    }
	for (size_t i = 0; i < clients.size(); ++i)
    {
        close(clients[i]->getFd());
        delete clients[i];
    }
    clients.clear();
    close(listenFd);
}