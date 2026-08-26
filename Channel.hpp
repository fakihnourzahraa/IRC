/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nour <nour@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 20:59:25 by miwehbe           #+#    #+#             */
/*   Updated: 2026/08/26 14:50:55 by nour             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <string>
#include <vector>
# include <iostream>
# include <fstream>
# include <iomanip>
# include <cstdlib>
# include "Client.hpp"
class Client;
class Channel
{
private:
    std::string name;//chan name
    std::string topic;//the chan topic
    std::vector<Client*> members;//pointer laan bde eml refer alclient exist
    std::vector<Client*> operators;//bteml store for client have priviliges bcz cmnd need privilige
    std::vector<Client*> invited;//store the client invited to client
    bool inviteOnly;//+i
    bool topicRestricted;//+t change topic
    bool hasKey;//+k has pass
    bool userLimitEnabled;//+l has max nbr of user
		//MODE chanel +l 10 /or/Mode chanel +k password
    std::string key;//for the pass
    size_t userLimit;

public:
    Channel(const std::string& name);//example chanel("42")
    ~Channel();
    const std::string& getName() const;
    bool isEmpty() const;
    //JOIN
    bool canJoin(Client* client, const std::string& key) const;
	//for join can join has the key is invited is their a limit 
    bool addMember(Client* client);
    bool isInvited(Client* client) const;
    //PART
    bool isMember(Client* client) const;//check it bcz some cmnd need to be as member(part,topic..)
    void removeMember(Client* client);//use for cmnd like kick,quit
    //PRIVMSG/NOTICE
    void broadcast(const std::string& message, Client* sender);//send it ro all without the sender wirh privmsg cmnd
    //TOPIC
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);
    bool getTopicRestricted() const;
    //INVITE
    void addInvite(Client* client);
    void removeInvite(Client* client);
    //KICK
    void kick(Client* client);//remove a client bcz operator kick it we cane call remove member inside it
    //MODE
    bool isOperator(Client* client) const;//know if he has the privilige
    //MODE +i / -i
    bool getInviteOnly() const;
    void setInviteOnly(bool enabled);
    //MODE +t / -t
    void setTopicRestricted(bool enabled);
    //MODE +k / -k
    bool getHasKey() const;
    const std::string& getKey() const;
    void setKey(const std::string& key);
    void removeKey();
    //MODE +o / -o
    void addOperator(Client* client);//make it as operator if we need
    void removeOperator(Client* client);//remove it Mode channel -o client
    //MODE +l / -l
    bool getUserLimitEnabled() const;
    size_t getUserLimit() const;
    void setUserLimit(size_t limit);
    void removeUserLimit();
    size_t getUserCount() const;
    bool channelFull() const;
};
#endif