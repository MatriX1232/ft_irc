/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 11:02:26 by root              #+#    #+#             */
/*   Updated: 2025/04/08 14:29:49 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"
#include "../include/Server.hpp"

Server::Server(int port) : _port(port)
{
    std::cout << "Created new SERVER" << std::endl;
}

Server::~Server()
{
    std::cout << "Destroyed SERVER" << std::endl;
}

int Server::listen(int n_clients)
{
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(this->_port);
 
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        std::cerr << "Error establishing the server socket" << std::endl;
        exit(0);
    }

    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, sizeof(servAddr));
    if(bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }
    
    ::listen(serverSd, n_clients);
    std::cout << "Listening on port " << _port << " for " << n_clients << " clients" << std::endl;

    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if(newSd < 0)
    {
        std::cerr << "Error accepting request from client!" << std::endl;
        exit(1);
    }
    this->_newSd = newSd;  
    this->_serverSd = serverSd;
    std::cout << 
        "Connected with client! | " <<
        "SD: " << newSd << " | " <<
        "IP: " << inet_ntoa(newSockAddr.sin_addr) << " | " <<
        "PORT: " << ntohs(newSockAddr.sin_port) << std::endl;

    return (0);
}

int Server::disconnect()
{
    try
    {
        close(this->_serverSd);
        ft_log("Server socket closed", "", 1);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error closing server socket: " << e.what() << std::endl;
        return (-1);
    }
    return (0);
}

Message Server::recv()
{
    char msg[150];

    std::cout << "Awaiting client message..." << std::endl;
    memset(&msg, 0, sizeof(msg));   //clear the buffer
    int bytesRead = ::recv(this->_newSd, (char*)&msg, sizeof(msg), 0);
    if (bytesRead == 0 || strcmp(msg, "exit") == 0)
        return Message();
    return (Message("Client", msg, std::time(0), bytesRead));
}

int Server::recv_file()
{
    std::string filename = this->recv().getContent() + "_RECV";

    std::ofstream file;
    file.open(filename.c_str(), std::ios::out);
    if(!file)
    {
        std::cout << "Error creating file" << std::endl;
        return -1;
    }
    char buffer[256];
    bzero(buffer, 256);
    int bytesReceived = 0;
    while((bytesReceived = ::recv(this->_newSd, buffer, 256, 0)) > 0)
    {
        file << buffer;
        bzero(buffer, 256);
    }
    std::cout << "File received" << std::endl;
    file.close();
    return 0;
}

bool Server::check_password(std::string password)
{
    return (this->_password == password);
}