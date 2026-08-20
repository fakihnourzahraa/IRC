#include "Client.hpp"

//ocf
Client::Client(int fd)
{
	this->fd = fd;
	this->nickname = "";
	this->username = "";
	this->realname = "";
	this->inputBuffer = "";
	this->outputBuffer = "";
	this->passwordAccepted = false;
	this->nicknameSet = false;
	this->usernameSet = false;
}
Client::~Client()
{}

//get
int Client::getFd() const
{
	return (this->fd);
}
const std::string& Client::getNickname() const
{
	return (this->nickname);
}
const std::string& Client::getUsername() const
{
	return (this->username);
}
const std::string& Client::getRealname() const
{
	return (this->realname);
}
//check
bool Client::isPasswordAccepted() const
{
	return (this->passwordAccepted);
}
bool Client::isNicknameSet() const
{
	return (this->nicknameSet);
}
bool Client::isUsernameSet() const
{
	return (this->usernameSet);
}
bool Client::isRegistered() const
{
	return (this->passwordAccepted && this->nicknameSet && this->usernameSet);
}
//set
void Client::setNickname(const std::string& nickname)
{
	this->nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	this->username = username;
}

void Client::setRealname(const std::string& realname)
{
	this->realname = realname;
}

void Client::setPasswordAccepted(bool accepted)
{
	this->passwordAccepted = accepted;
}

void Client::setNicknameSet(bool set)
{
	this->nicknameSet = set;
}
void Client::setUsernameSet(bool set)
{
	this->usernameSet = set;
}
//input
void Client::appendInput(const std::string& data)
{
	this->inputBuffer += data;//akid+= bcz tcp splitt
}
bool Client::hasCompleteLine() const
{
	return (this->inputBuffer.find("\r\n") != std::string::npos);//npos yane no position found
				//so true if complete cmnd and \r\n found false if npos
}

std::string Client::extractLine()
{
	size_t pos;
	std::string line;

	pos = this->inputBuffer.find("\r\n");//find the frst \r\n first cmnd so its contain it position
	if (pos == std::string::npos)
		return ("");//if no \r\n found return "" no cmnd found

	line = this->inputBuffer.substr(0, pos);//substring until the position until\r\n
	this->inputBuffer.erase(0, pos + 2);//remove it +2 laan \r hye 1 w \n hye 2

	return (line);
}//yane mnkhud lcmnd mn imputbuff return it and remove it from input buff

//output
void Client::appendOutput(const std::string& data)
{
	this->outputBuffer += data;
}

const std::string& Client::getOutputBuffer() const
{
	return (this->outputBuffer);
}//data current wait to send

bool Client::hasOutput() const
{
	return (!this->outputBuffer.empty());
}//to check enu is there any outpu to send it to client

void Client::removeSentData(size_t bytes)
{
	if (bytes >= this->outputBuffer.size())
		this->outputBuffer.clear();
	else
		this->outputBuffer.erase(0, bytes);
}//remove it from outp if ir send but send it depend on bytes size


/*
so basically this file has 3 main respon
1- store and check the client information "check only does not validate"
2-store the data from the client to the server in inputBuffer ,then try
to split the command depend on \r\n  store it in line then remove it(recieve data,split line)
3-store outgoing data from server to client (outputBuffer,data to send)
*/