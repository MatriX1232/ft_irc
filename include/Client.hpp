/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 23:47:53 by root              #+#    #+#             */
/*   Updated: 2025/06/27 16:40:18 by root             ###   ########.fr       */
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
		bool            _authenticated;
		std::string		_currentChannel;
		std::string     _nickname;
		std::string		_realName;
		std::string		_username;

	public:
		Client();
		Client(int sd, std::string ip, int port);
		Client(const Client& other) = delete;
		Client& operator=(const Client& other) = delete;
		Client(Client&&) noexcept = default;
		Client& operator=(Client&&) noexcept = default;
		~Client();
		
		
		int         getSd() const;
		std::string getIp() const;
		int         getPort() const;
		int			getListenFd() const;
		int			getFd() const;
		std::string getCurrentChannel();
		std::string getNickname() const;
		std::string getRealName() const;
		std::string getUsername() const;

		bool	isAuthenticated() const;
		
		void	setRealName(const std::string& realName);
		void	setUsername(const std::string& username);
		void	setNickname(const std::string& nickname);
		void	setAuthenticated(bool authenticated);
		void	setSd(int sd);
		void	setIp(const std::string& ip);
		void	setPort(int port);
		void	setCurrentChannel(const std::string& channel);
		bool	isEmpty() const;

		friend std::ostream &operator<<(std::ostream& os, const Client& client);
		friend bool operator==(const Client &scr, const Client &other);
};

#endif