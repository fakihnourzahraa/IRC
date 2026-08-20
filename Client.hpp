/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miwehbe <miwehbe@student.42beirut.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 20:05:15 by miwehbe           #+#    #+#             */
/*   Updated: 2026/08/18 20:05:15 by miwehbe          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include<string>
class Client
{
	private:
		int fd;//each one has fd
		std::string nickname;//for nick cmnd public irc name
		std::string username;//not necc as nickname used by user cmnd other normally dont call with this
		std::string realname;//also used by user cmnd
							//USER <username> <mode> <unused>:<realname>
		std::string inputBuffer;//store data from client to server
		std::string outputBuffer;//from the  server to client 
		bool passwordAccepted;
		bool nicknameSet;
		bool usernameSet;
		Client(const Client& other);//pvt bcz we have a pvt fc for each cliebnt so we cant make it public
		Client& operator=(const Client& other);

	public:
		//ocf
		Client(int fd);
		~Client();
		//get
		int                 getFd() const;
		const std::string&  getNickname() const;
		const std::string&  getUsername() const;
		const std::string&  getRealname() const;
		//check
		bool isPasswordAccepted() const;
		bool isNicknameSet() const;
		bool isUsernameSet() const;
		bool isRegistered() const;
		//set
		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setRealname(const std::string& realname);
		void setPasswordAccepted(bool accepted);
		void setNicknameSet(bool set);
		void setUsernameSet(bool set);
		//input
		void appendInput(const std::string& data);//we need it bcz tcp can split the cmnd so we make sure that
													//we append the cmmd roght
		bool hasCompleteLine() const;//make sure it a right cmnd end with /r/n
		std::string extractLine();//we use it if we have many cmnd in one line so we split
									//each one and run it then remove it from inputbuff
		//output
		void appendOutput(const std::string& data);//same as before but from server tothe client in outputbuffer
		const std::string& getOutputBuffer() const;
		bool hasOutput() const;//check if there also thing to send
		void removeSentData(size_t bytes);

};

#endif