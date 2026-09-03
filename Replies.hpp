#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

class Replies
{
public:
	//registration
	static std::string welcome(const std::string& nickname);//001
	static std::string yourHost(const std::string& nickname);//002
	static std::string created(const std::string& nickname);//003
	static std::string myInfo(const std::string& nickname);//004
	//errors
	static std::string noSuchNick(const std::string& nickname,const std::string& target);//401
	static std::string noSuchChannel(const std::string& nickname,const std::string& channel);//403
	static std::string noOrigin(const std::string& nickname);//409 for ping pong
	static std::string noRecipient(const std::string& nickname,const std::string& command);//411
	static std::string noTextToSend(const std::string& nickname);//412
	static std::string cannotSendToChannel(const std::string& nickname,const std::string& channel);//404
	static std::string unknownCommand(const std::string& nickname,const std::string& command);//421
	static std::string noNicknameGiven(const std::string& nickname);//431
	static std::string erroneousNickname(const std::string& nickname,const std::string& badNickname);//432
	static std::string nicknameInUse(const std::string& nickname,const std::string& badNickname);//433
	static std::string needMoreParams(const std::string& nickname,const std::string& command);//461
	static std::string alreadyRegistered(const std::string& nickname);//462
	static std::string passwordMismatch(const std::string& nickname);//464
	static std::string notRegistered(const std::string& nickname);//451
	//channel/user errors
	static std::string userNotInChannel(const std::string& nickname,const std::string& target,const std::string& channel);//441
	static std::string notOnChannel(const std::string& nickname,const std::string& channel);//442
	static std::string userOnChannel(const std::string& nickname,const std::string& target,const std::string& channel);//443
	//JOIN / MODE errors
	static std::string channelIsFull(const std::string& nickname,const std::string& channel);//471
	static std::string unknownMode(const std::string& nickname,const std::string& mode);//472
	static std::string inviteOnlyChannel(const std::string& nickname,const std::string& channel);//473
	static std::string badChannelKey(const std::string& nickname,const std::string& channel);//475
	static std::string chanOpPrivilegesNeeded(const std::string& nickname,const std::string& channel);//482
	//TOPIC / NAMES
	static std::string noTopic(const std::string& nickname,const std::string& channel);//331
	static std::string topic(const std::string& nickname,const std::string& channel,const std::string& topicText);//332
	static std::string namesReply(const std::string& nickname,const std::string& channel,const std::string& names);//353
	static std::string endOfNames(const std::string& nickname,const std::string& channel);//366
	static std::string inviting(const std::string& nickname, const std::string& target, const std::string& channel);//341
private:
	Replies();
	~Replies();
	static std::string prefix(const std::string& nickname,const std::string& code);
};

#endif