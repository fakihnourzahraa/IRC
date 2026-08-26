# include "Channel.hpp"

Channel::Channel(const std::string& newName)
{
    name = newName;
}

Channel::~Channel()
{
	std::cout <<"Default destructor is called" << std::endl;
}
const std::string& Channel::getName() const
{
    return (this->name);
}
bool Channel::isEmpty() const
{
    return (true); //for later
}

    
bool Channel::canJoin(Client* client, const std::string& key) const
{
    if (inviteOnly)
    {
        if (!isInvited(client))
            return false;
        else
        {
            if (key != this->key)
                return false;
            if (channelFull())
                return false;
        }
        return true;
    }
    else
    {
        if (channelFull())
            return false;
    }
}

bool Channel::channelFull() const
{
    if (userLimitEnabled)
    {
        if (getUserLimit() == getUserCount())
            return true;
    }
    return false;
}

bool Channel::addMember(Client* client)
{
    // if memember is already in
    members.push_back(client);
    return true;
}
bool Channel::isInvited(Client* client) const
{
    for (std::vector<Client*>::const_iterator it = invited.begin(); it != invited.end(); ++it)
    {
        if (*it == client)
            return true;
    }
    return false;
}
bool Channel::isMember(Client* client) const
{
    for (std::vector<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (*it == client)
            return true;
    }
    return false;
}
void Channel::removeMember(Client* client) 
{
    for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it)
    {
        if (*it == client)
        {    members.erase(it);
            return ;}
    }
}
// void Channel::broadcast(const std::string& message, Client* sender)
// {

// }


// const std::string& Channel::getTopic() const
// {

// }
//     void setTopic(const std::string& topic);
//     bool getTopicRestricted() const;
//     //INVITE
//     void addInvite(Client* client);
//     void removeInvite(Client* client);
//     //KICK
//     void kick(Client* client);//remove a client bcz operator kick it we cane call remove member inside it
//     //MODE
//     bool isOperator(Client* client) const;//know if he has the privilige
//     //MODE +i / -i
//     bool getInviteOnly() const;
//     void setInviteOnly(bool enabled);
//     //MODE +t / -t
//     void setTopicRestricted(bool enabled);
//     //MODE +k / -k
//     bool getHasKey() const;
//     const std::string& getKey() const;
//     void setKey(const std::string& key);
//     void removeKey();
//     //MODE +o / -o
//     void addOperator(Client* client);//make it as operator if we need
//     void removeOperator(Client* client);//remove it Mode channel -o client
//     //MODE +l / -l
//     bool getUserLimitEnabled() const;
//     size_t getUserLimit() const;
//     void setUserLimit(size_t limit);
//     void removeUserLimit();
//     size_t getUserCount() const;
//     bool channelFull() const;