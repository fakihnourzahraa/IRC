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
class CommandHandler;
class Server
{
private:
    int port;
    std::string password;
    int listenFd;

    std::vector<Client*> clients;
    std::vector<Channel*> channels;
    std::vector<struct pollfd> pollFds;
	CommandHandler* commandHandler;
    //setup
    void setupSocket();

    void acceptNewClient();
    void receiveData(Client& client);
    void sendData(Client& client);
    //management
    Channel* findChannel(const std::string& name);
	Client* findClientByFd(int fd);

public:
    Server(int port, const std::string& password);
    ~Server();
    void run();
	const std::string& getPassword() const;
	void removeClient(Client& client);
	Client* findClientByNickname(const std::string& nickname);
};

#endif