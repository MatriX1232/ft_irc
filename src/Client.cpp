/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 15:06:38 by root              #+#    #+#             */
/*   Updated: 2025/04/18 22:50:48 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include "../include/Message.hpp"
#include <iostream>
#include <string>

Client::Client(int sd, std::string ip, int port) : _clientSd(sd), _ip(ip), _port(port), _authenticated(false)
{
    std::cout << "Created new CLIENT" << std::endl;
}

Client::Client() : _clientSd(-1), _ip(""), _port(0), _authenticated(false)
{
    std::cout << "Created new CLIENT" << std::endl;
}

Client::Client(const Client& other)
{
    *this = other;
}

Client& Client::operator=(const Client& other)
{
    if (this != &other)
    {
        this->_clientSd = other._clientSd;
        this->_ip = other._ip;
        this->_port = other._port;
        this->_nickname = other._nickname;
        this->_authenticated = other._authenticated;
    }
    return *this;
}

Client::~Client()
{
    // std::cout << "Destroyed CLIENT" << std::endl;
}

int Client::getSd() const
{
    return this->_clientSd;
}

std::string Client::getIp() const
{
    return this->_ip;
}

int Client::getPort() const
{
    return this->_port;
}

std::string Client::getNickname() const
{
    return this->_nickname;
}

int Client::getListenFd() const
{
    return this->_clientSd;
}

int Client::getFd() const
{
    return this->_clientSd;
}

bool Client::isAuthenticated() const
{
    return this->_authenticated;
}

void Client::setNickname(std::string nickname)
{
    this->_nickname = nickname;
}

void Client::setAuthenticated(bool authenticated)
{
    this->_authenticated = authenticated;
}

void Client::setSd(int sd)
{
    this->_clientSd = sd;
}

void Client::setIp(std::string ip)
{
    this->_ip = ip;
}

void Client::setPort(int port)
{
    this->_port = port;
}

std::ostream &operator<<(std::ostream& os, const Client& client)
{
    os << "----------------------------------------" << std::endl;
    os << BOLD << "Client Information:" << END << std::endl;
    os << "Client: " << client._nickname << std::endl;
    os << "IP: " << client._ip << std::endl;
    os << "Port: " << client._port << std::endl;
    os << "Socket Descriptor: " << client._clientSd << std::endl;
    os << "Authenticated: " << (client._authenticated ? "true" : "false") << std::endl;
    os << "----------------------------------------" << std::endl << std::endl;

    return os;
}

bool Client::isEmpty()
{
    return this->_clientSd == -1;
}