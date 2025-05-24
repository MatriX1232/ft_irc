/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 23:47:53 by root              #+#    #+#             */
/*   Updated: 2025/05/24 21:03:37 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #pragma ones

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

class Client
{
	private:
		int             _clientSd;
		std::string     _ip;
		int             _port;
		char            _msg[150];
		std::string     _nickname;
		bool            _authenticated;
		std::string		_currentChannel;
	
	public:
		Client();
		Client(int sd, std::string ip, int port);
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();
		
		int         getSd() const;
		std::string getIp() const;
		int         getPort() const;
		std::string getNickname() const;
		int			getListenFd() const;
		int			getFd() const;
		std::string getCurrentChannel();
		
		bool	isAuthenticated() const;
		
		void setNickname(std::string nickname);
		void setAuthenticated(bool authenticated);
		void setSd(int sd);
		void setIp(std::string ip);
		void setPort(int port);
		void setCurrentChannel(std::string channel);
		bool    isEmpty();
		
		friend std::ostream &operator<<(std::ostream& os, const Client& client);
		friend bool operator==(const Client &scr, const Client &other);
};

#endif