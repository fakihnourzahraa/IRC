# include "Server.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "CommandHandler.hpp"
# include <sys/socket.h>//socket,bind,listen...
# include <unistd.h>//close
# include <fcntl.h>//for fcntl
# include <iostream>//cout,cerr
# include <stdexcept>//exception
# include <cstring>//for memset
# include <sstream>//string stream
# include <cctype>//toupper...
# include <netinet/in.h>//sockaddrs_in
# include <csignal>

Server::Server(int portNumber, const std::string& serverPassword)
{
    port = portNumber;
    password = serverPassword;
    listenFd = -1;//mean no valid socket,bcz socket wasnt created yet
    commandHandler = new CommandHandler(*this);//it let me access the server and find client ,channel...
}//the constructor we recieve the port nbr from main and we store it
//so here we save the port,save the pass,no socket first put(-1),create the cmnd handler and give it acces to server

Server::~Server()
{
	size_t i;
	for (i = 0; i < clients.size(); i++)
	{
		close(clients[i]->getFd());//close it from os
		delete clients[i];//bcz we use new
	}
	for (i = 0; i < channels.size(); i++)
		delete channels[i];
	if (listenFd != -1)
		close(listenFd);
	delete commandHandler;
}//its clean everything the server created(avoid memory leak)


void Server::handleSignal(int signum)
{
    (void)signum;
}

void Server::setupSocket()
{
	//socket->setsockopt->bind->listen->fcntl(non blocking)->signalhandle
    listenFd = socket(AF_INET, SOCK_STREAM, 0);//create a socket ipv4,tcp,default protocol
    if (listenFd < 0)
        throw std::runtime_error("socket() failed");
    int opt = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//configure the socket
	//reuseaddr rge most important bcz it let me to reuse the add/port after we stop the server and run it immediately

    struct sockaddr_in address;//we create the structure that contain info for bind()(addr,ip family,port)
    std::memset(&address, 0, sizeof(address));//initial to 0 to make sure we do right
    address.sin_family = AF_INET;//ipv4
    address.sin_addr.s_addr = INADDR_ANY;//accept connection from any local ip addrs
    address.sin_port = htons(port);//transform from computer byte to network byte

    if (bind(listenFd, (struct sockaddr*)&address, sizeof(address)) < 0)
        throw std::runtime_error("bind() failed");

    if (listen(listenFd, 128) < 0)//128 pending request not 128 client
        throw std::runtime_error("listen() failed");

    fcntl(listenFd, F_SETFL, O_NONBLOCK);//nonblock make recv send return immediately instead of blocking
	struct sigaction sa;//struct for the signal
    sa.sa_handler = Server::handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    struct pollfd pfd;//server use poll() we need to give it info about the fd we want to monitor
	//which fd,what event are we handle,what event actually happen

    pfd.fd = listenFd;
    pfd.events = POLLIN;//tell me when fd is ready to read ,for listenning fd pollin mean new client connection is waiting
    pfd.revents = 0;//we initial by 0 then poll overwrite it
	//event :what we want to watch,revent :what actually happened
    pollFds.push_back(pfd);//add the listen fd to the vector
}

void Server::acceptNewClient()
{
    struct sockaddr_in clientAddress;//create struc where accept will store info about client(ip,port)
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
    Client* client = new Client(clientFd);//fd,nickname,username...
    clients.push_back(client);//we push it to client vector
	
    struct pollfd pfd;//create a pollfd for the new client
	//poll() need pollfd for each client
    pfd.fd = clientFd;//tell poll that we work by the clientfd
    pfd.events = POLLIN;//to know when this client have data avail to read
    pfd.revents = 0;
    pollFds.push_back(pfd);
}
//client want to connect->poll()detect pollin on listenfd->accpetnewclient()->accept()->
//new client socket fd created->make client non block->create client obj->add client to vect and fd to vect

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
    //check if inputbuff have at least an irc cmnd end with\r\n
    while (findClientByFd(fd) != NULL && client.hasCompleteLine())
    {
        std::string line = client.extractLine();//take one input out of inputbuff
        commandHandler->dispatchCommand(client, line);//identify the command 
    }
}//recv->put rec byte into inputbuffer->check for the complete cmnd->extract one comnd->dispatchcmnd

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
    int fd = client.getFd();//save the client fd we want to delete
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
        //check if the current chanel is the ome we search for
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
            delete *it;//free the objct
            channels.erase(it);//remove pointer from the vector
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
        if (ready < 0)//if the poll fail
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
//setupsocket->wait with poll()->something happen
//if new client->accept()
//else it is an existing client->receive data->execute cmnd->send response ->go back to poll()
//poll allow monitor multiple socket at the same time
//events contains what we want to monitor, revents contains what actually happened after poll() returns