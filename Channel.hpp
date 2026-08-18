/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miwehbe <miwehbe@student.42beirut.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 20:59:25 by miwehbe           #+#    #+#             */
/*   Updated: 2026/08/18 20:59:25 by miwehbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

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
    const std::string& getTopic() const;

    bool isMember(Client* client) const;//check it bcz some cmnd need to be as member(part,topic..)
    bool addMember(Client* client);
    void removeMember(Client* client);//use for cmnd like kick,quit

    bool isOperator(Client* client) const;//know if he has the privilige
    void addOperator(Client* client);//make it as operator if we need
    void removeOperator(Client* client);//remove it Mode channel -o client

    void broadcast(const std::string& message, Client* sender);//send it ro all without the sender wirh privmsg cmnd
    void setTopic(const std::string& topic);

    void addInvite(Client* client);
    void removeInvite(Client* client);
    bool isInvited(Client* client) const;
    bool canJoin(Client* client, const std::string& key) const;
	//for join can join has the key is invited is their a limit 
    bool getInviteOnly() const;
    bool getTopicRestricted() const;
    bool getHasKey() const;
    bool getUserLimitEnabled() const;

    void setInviteOnly(bool enabled);
    void setTopicRestricted(bool enabled);
    void setKey(const std::string& key);
    void removeKey();
    void setUserLimit(size_t limit);
    void removeUserLimit();

    const std::string& getKey() const;
    size_t getUserLimit() const;
    void kick(Client* client);//remove a client bcz operator kick it we cane call remove member inside it
    bool isEmpty() const;
};

#endif