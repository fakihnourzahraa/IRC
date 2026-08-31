# include "Channel.hpp"

Channel::Channel(const std::string& newName)
    : name(newName), inviteOnly(false), topicRestricted(false),
      hasKey(false), userLimitEnabled(false), userLimit(0)
{
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
    if (members.size() == 0)
        return true;
    return false;
}

bool Channel::canJoin(Client* client, const std::string& key) const
{
    if (inviteOnly && !isInvited(client))
        return false;
    if (hasKey && key != this->key)
        return false;
    if (channelFull())
        return false;
    return true;
}

bool Channel::channelFull() const
{
    if (userLimitEnabled)
    {
        if (getUserLimit() <= getUserCount())
            return true;
    }
    return false;
}

bool Channel::addMember(Client* client)
{
    if (channelFull())
        return false;
    for (std::vector<Client*>::iterator i= members.begin(); i!= members.end(); ++i)
    {
        if (*i== client)
            return false;
    }
    members.push_back(client);
    return true;
}
bool Channel::isInvited(Client* client) const
{
    for (std::vector<Client*>::const_iterator i = invited.begin(); i != invited.end(); ++i)
    {
        if (*i == client)
            return true;
    }
    return false;
}
bool Channel::isMember(Client* client) const
{
    for (std::vector<Client*>::const_iterator i= members.begin(); i!= members.end(); ++i)
    {
        if (*i== client)
            return true;
    }
    return false;
}
void Channel::removeMember(Client* client)
{
    for (std::vector<Client*>::iterator i= members.begin(); i!= members.end(); ++i)
    {
        if (*i== client)
        {    members.erase(i);
            return ;}
    }
}

void Channel::broadcast(const std::string& message, Client* sender)
{
    for (std::vector<Client*>::iterator i = members.begin(); i != members.end(); ++i)
    {
        if (*i != sender)
            (*i)->appendOutput(message);
    }
}

const std::string& Channel::getTopic() const
{
    return this->topic;
}
void Channel::setTopic(const std::string& topic)
{
    this->topic = topic;
}
bool Channel::getTopicRestricted() const
{
    return topicRestricted;
}

void Channel::addInvite(Client* client)
{
    for (std::vector<Client*>::iterator i= invited.begin(); i!= invited.end(); ++i)
    {
        if (*i== client)
            return;
    }
    invited.push_back(client);
}
void Channel::removeInvite(Client* client)
{
    for (std::vector<Client*>::iterator i= invited.begin(); i!= invited.end(); ++i)
    {
        if (*i== client)
        {    invited.erase(i);
            return ;}
    }   
}

void Channel::kick(Client* client)
{
    removeMember(client);
    removeInvite(client);
    removeOperator(client);
}
bool Channel::isOperator(Client* client) const
{
   for (std::vector<Client*>::const_iterator i = operators.begin(); i != operators.end(); ++i)
    {
        if (*i == client)
            return true;
    }
    return false;
}

bool Channel::getInviteOnly() const
{
    return inviteOnly;
}
void Channel::setInviteOnly(bool enabled)
{
    this->inviteOnly = enabled;
}

void Channel::setTopicRestricted(bool enabled)
{
    this->topicRestricted = enabled;
}

bool Channel::getHasKey() const
{
    return hasKey;
}
const std::string& Channel::getKey() const
{
    return this->key;
}
void Channel::setKey(const std::string& key)
{
    this->key = key;
    this->hasKey = true;
}
void Channel::removeKey()
{
    this->key = "";
    this->hasKey = false;
}

void Channel::addOperator(Client* client)
{
    for (std::vector<Client*>::iterator i= operators.begin(); i!= operators.end(); ++i)
    {
        if (*i== client)
            return;
    }
    operators.push_back(client);
}
void Channel::removeOperator(Client* client)
{
    for (std::vector<Client*>::iterator i= operators.begin(); i!= operators.end(); ++i)
    {
        if (*i== client)
        {    operators.erase(i);
            return ;}
    }   
}
bool Channel::getUserLimitEnabled() const
{
    return userLimitEnabled;
}
size_t Channel::getUserLimit() const
{
    return userLimit;
}
void Channel::setUserLimit(size_t limit)
{
    this->userLimit = limit;
    userLimitEnabled = true;
}
void Channel::removeUserLimit()
{
    this->userLimit = 0;
    userLimitEnabled = false;
}
size_t Channel::getUserCount() const
{
   return members.size();
}

const std::vector<Client*>& Channel::getMembers() const
{
    return members;
}
const std::vector<Client*>& Channel::getOperators() const
{
    return operators;
}
