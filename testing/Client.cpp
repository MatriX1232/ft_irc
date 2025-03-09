/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 00:05:25 by root              #+#    #+#             */
/*   Updated: 2025/03/09 15:31:07 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(std::string ip, int port) : _port(port), _ip(ip)
{
    std::cout << "Created new CLIENT" << std::endl;
}

Client::~Client()
{
    std::cout << "Destroyed CLIENT" << std::endl;
}

int Client::try_connect()
{
    struct hostent* host = gethostbyname(this->_ip.c_str());
    if(host == NULL)
    {
        std::cout << "Error: Host not found" << std::endl;
        return -1;
    } 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(this->_port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);

    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        std::cout<<"Error connecting to socket!"<<std::endl;
        return -1;
    }
    std::cout << "Connected to the server!" << std::endl;
    return 0;
}

int Client::disconnect()
{
    close(this->_clientSd);
    std::cout << "Disconnected from the server!" << std::endl;
    return 0;
}

int Client::send(std::string msg)
{
    char _msg[150];
    bzero(_msg, 150);
    strcpy(_msg, msg.c_str());
    // :: is used to call the global function send | not the member function send
    int status = ::send(this->_clientSd, (char*)&_msg, 150, 0);
    if(status < 0)
    {
        std::cout << "Error sending message" << std::endl;
        return -1;
    }
    std::cout << "Message sent..." << std::endl;
    return 0;
}

int Client::recv()
{
    bzero(this->_msg, 150);
    long status = ::recv(this->_clientSd, (char*)this->_msg, sizeof(this->_msg), 0);
    if(status < 0)
    {
        std::cout << "Error receiving message" << std::endl;
        return -1;
    }
    std::cout << "Message received: " << this->_msg << std::endl;
    return 0;
}

