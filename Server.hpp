/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miwehbe <miwehbe@student.42beirut.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 14:29:07 by miwehbe           #+#    #+#             */
/*   Updated: 2026/08/20 14:29:07 by miwehbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>

class Client;
class Channel;

class Server
{
private:
    int port;
    std::string password;
    int listenFd;

    std::vector<Client*> clients;
    std::vector<Channel*> channels;
    std::vector<struct pollfd> pollFds;
    //setup
    void setupSocket();
    //Connection handling
    void acceptNewClient();
    void receiveData(Client& client);
    void sendData(Client& client);
    void removeClient(Client& client);
    //management
    Channel* findChannel(const std::string& name);
	Client* findClientByFd(int fd);
	Client* findClientByNickname(const std::string& nickname);
    // Command dispatch
    void dispatchCommand(Client& client, const std::string& line);
    //IRC commands
    void handlePass(Client& client,const std::vector<std::string>& param);
    void handleNick(Client& client,const std::vector<std::string>& param);
    void handleUser(Client& client,const std::vector<std::string>& param);
    void handleJoin(Client& client,const std::vector<std::string>& param);
    void handlePart(Client& client,const std::vector<std::string>& param);
    void handlePrivmsg(Client& client,const std::vector<std::string>& param);
    void handleNotice(Client& client,const std::vector<std::string>& param);
    void handleTopic(Client& client,const std::vector<std::string>& param);
    void handleInvite(Client& client,const std::vector<std::string>& param);
    void handleKick(Client& client,const std::vector<std::string>& param);
    void handleMode(Client& client,const std::vector<std::string>& param);
    void handleQuit(Client& client,const std::vector<std::string>& param);
	void handlePing(Client& client, const std::vector<std::string>& param);

public:
    Server(int port, const std::string& password);
    ~Server();
    void run();
};

#endif