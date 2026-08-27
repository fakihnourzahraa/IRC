#include "Replies.hpp"

Replies::Replies(){}
Replies::~Replies(){}
std::string Replies::prefix(const std::string& nickname,const std::string& code)
{
	std::string target = nickname;

	if (target.empty())
		target = "*";
	return (":ircserv " + code + " " + target + " ");
}


std::string Replies::welcome(const std::string& nickname)
{
	return (prefix(nickname, "001")+ ":Welcome to the IRC Network\r\n");
}//:ircserv 001 Mira :Welcome to the IRC Network
std::string Replies::yourHost(const std::string& nickname)
{
	return (prefix(nickname, "002")+ ":Your host is ircserv\r\n");
}//:ircserv 002 Mira :Your host is ircserv
std::string Replies::created(const std::string& nickname)
{
	return (prefix(nickname, "003")+ ":This server was created today\r\n");
}//:ircserv 003 Mira :This server was created today
std::string Replies::myInfo(const std::string& nickname)
{
	return (prefix(nickname, "004")+ "ircserv 1.0 i itkol\r\n");
}//:ircserv 004 Mira ircserv 1.0 i itkol


std::string Replies::noSuchNick(const std::string& nickname,const std::string& target)
{
	return (prefix(nickname, "401")+ target + " :No such nick/channel\r\n");
}//:ircserv 401 Mira lili :No such nick/channel(send a msg to user not exist lili)
std::string Replies::noSuchChannel(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "403")+ channel + " :No such channel\r\n");
}//:ircserv 403 Mira #ss :No such channel(when do join for chanel not exist)
std::string Replies::noOrigin(const std::string& nickname)
{
	return (prefix(nickname, "409")+ ":No origin specified\r\n");
}//:ircserv 409 Mira :No origin specified(PING sent with no token)
std::string Replies::noRecipient(const std::string& nickname,const std::string& command)
{
	return (prefix(nickname, "411")+ ":No recipient given (" + command + ")\r\n");
}//in privmsg we need <target>:<message> so when no one specify 411
//privmsg can be broadcast to a channel and can specific to one so if dont do one of them it gave this error
std::string Replies::noTextToSend(const std::string& nickname)
{
	return (prefix(nickname, "412")+ ":No text to send\r\n");
}//:ircserv 412 Mira :No text to send
std::string Replies::cannotSendToChannel(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "404")+ channel + " :Cannot send to channel\r\n");
}//:ircserv 404 Mira #42 :Cannot send to channel(user not allowed to send)
std::string Replies::unknownCommand(const std::string& nickname,const std::string& command)
{
	return (prefix(nickname, "421")+ command + " :Unknown command\r\n");
}//:ircserv 421 Mira HELLO :Unknown command(hello not known)
std::string Replies::noNicknameGiven(const std::string& nickname)
{
	return (prefix(nickname, "431")+ ":No nickname given\r\n");
}//:ircserv 431 Mira :No nickname given (if we dont gave him nickame)
std::string Replies::erroneousNickname(const std::string& nickname,const std::string& badNickname)
{
	return (prefix(nickname, "432")+ badNickname + " :Erroneous nickname\r\n");
}//:ircserv 432 Mira !!! :Erroneous nickname(like !!!!)
std::string Replies::nicknameInUse(const std::string& nickname,const std::string& badNickname)
{
	return (prefix(nickname, "433")+ badNickname + " :Nickname is already in use\r\n");
}
std::string Replies::needMoreParams(const std::string& nickname,const std::string& command)
{
	return (prefix(nickname, "461")+ command + " :Not enough parameters\r\n");
}//:ircserv 461 Mira JOIN :Not enough parameters
std::string Replies::alreadyRegistered(const std::string& nickname)
{
	return (prefix(nickname, "462")+ ":You may not reregister\r\n");
}
std::string Replies::passwordMismatch(const std::string& nickname)
{
	return (prefix(nickname, "464")+ ":Password incorrect\r\n");
}


std::string Replies::userNotInChannel(const std::string& nickname,const std::string& target,const std::string& channel)
{
	return (prefix(nickname, "441")+ target + " " + channel
		+ " :They aren't on that channel\r\n");
}
std::string Replies::notOnChannel(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "442")
		+ channel + " :You're not on that channel\r\n");
}
std::string Replies::userOnChannel(const std::string& nickname,const std::string& target,const std::string& channel)
{
	return (prefix(nickname, "443")+ target + " " + channel
		+ " :is already on channel\r\n");
}


std::string Replies::channelIsFull(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "471")
		+ channel + " :Cannot join channel (+l)\r\n");
}
std::string Replies::unknownMode(const std::string& nickname,const std::string& mode)
{
	return (prefix(nickname, "472")
		+ mode + " :is unknown mode char to me\r\n");
}
std::string Replies::inviteOnlyChannel(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "473")
		+ channel + " :Cannot join channel (+i)\r\n");
}
std::string Replies::badChannelKey(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "475")
		+ channel + " :Cannot join channel (+k)\r\n");
}
std::string Replies::chanOpPrivilegesNeeded(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "482")
		+ channel + " :You're not channel operator\r\n");
}


std::string Replies::noTopic(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "331")
		+ channel + " :No topic is set\r\n");
}
std::string Replies::topic(const std::string& nickname,const std::string& channel,const std::string& topicText)
{
	return (prefix(nickname, "332")
		+ channel + " :" + topicText + "\r\n");
}
std::string Replies::namesReply(const std::string& nickname,const std::string& channel,const std::string& names)
{
	return (prefix(nickname, "353")
		+ "= " + channel + " :" + names + "\r\n");
}
std::string Replies::endOfNames(const std::string& nickname,const std::string& channel)
{
	return (prefix(nickname, "366")
		+ channel + " :End of /NAMES list\r\n");
}