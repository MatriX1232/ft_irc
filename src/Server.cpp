/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/09 11:02:26 by root              #+#    #+#             */
/*   Updated: 2025/04/18 22:52:39 by root             ###   ########.fr       */
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

int Server::start_listening(int n_clients)
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

    // Set socket to non-blocking
    int flags = fcntl(serverSd, F_GETFL, 0);
    fcntl(serverSd, F_SETFL, flags | O_NONBLOCK);

    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, sizeof(servAddr));
    if(bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }

    ::listen(serverSd, n_clients);
    std::cout << "Listening on port " << _port << " for " << n_clients << " clients" << std::endl;

    this->_serverSd = serverSd;
    return 0;
}

// ...existing code...
int Server::accept_new_client()
{
    struct pollfd pfd;
    pfd.fd = this->_serverSd;
    pfd.events = POLLIN;

    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    bzero(&newSockAddr, sizeof(newSockAddr));

    int poll_res = poll(&pfd, 1, -1); // Wait indefinitely for an event
    if (poll_res < 0)
    {
        std::cerr << "Poll error on server socket" << std::endl;
        return -1;
    }
    if (pfd.revents & POLLIN)
    {
        int newSd = accept(this->_serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
        if (newSd < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                return 0; // No new connection yet
            std::cerr << "Error accepting request from client!" << std::endl;
            return -1;
        }

        // Make the new client socket non-blocking
        int flags = fcntl(newSd, F_GETFL, 0);
        if (flags < 0)
        {
            std::cerr << "Error getting flags on new socket: " << strerror(errno) << std::endl;
            close(newSd);
            return -1;
        }
        if (fcntl(newSd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            std::cerr << "Error setting non-blocking on new socket: " << strerror(errno) << std::endl;
            close(newSd);
            return -1;
        }

        this->_newSd = newSd;
        std::string ip = inet_ntoa(newSockAddr.sin_addr);
        int port = ntohs(newSockAddr.sin_port);
        Client newClient(newSd, ip, port);
        this->_clients.push_back(newClient);
        std::cout <<
            "Connected with client! | " <<
            "SD: " << newSd << " | " <<
            "IP: " << ip << " | " <<
            "PORT: " << port << std::endl;
    }
    return 0;
}
// ...existing code...

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

Message Server::recv(Client &client)
{
    char msg[150];

    // std::cout << "Awaiting client message..." << std::endl;
    memset(&msg, 0, sizeof(msg));   //clear the buffer
    int bytesRead = ::recv(client.getSd(), (char*)&msg, sizeof(msg), 0);
    if (bytesRead == 0 || strcmp(msg, "exit") == 0)
        return Message();
    else
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            std::cout << "No data available on non-blocking socket" << std::endl;
        else
            std::cerr << "Error receiving data from client" << std::endl;
    }
    return (Message("Client", msg, std::time(0), bytesRead));
}

// int Server::recv_file()
// {
//     std::string filename = this->recv().getContent() + "_RECV";

//     std::ofstream file;
//     file.open(filename.c_str(), std::ios::out);
//     if(!file)
//     {
//         std::cout << "Error creating file" << std::endl;
//         return -1;
//     }
//     char buffer[256];
//     bzero(buffer, 256);
//     int bytesReceived = 0;
//     while((bytesReceived = ::recv(this->_newSd, buffer, 256, 0)) > 0)
//     {
//         file << buffer;
//         bzero(buffer, 256);
//     }
//     std::cout << "File received" << std::endl;
//     file.close();
//     return 0;
// }

bool Server::check_password(std::string password)
{
    return (this->_password == password);
}

int Server::get_port()
{
    return this->_port;
}

int Server::get_serverSd()
{
    return this->_serverSd;
}

int Server::getListenFd() const
{
    return this->_serverSd;
}

int Server::getFd() const
{
    return this->_serverSd;
}

std::vector<Client> Server::get_clients() const
{
    return this->_clients;
}

std::ostream &operator<<(std::ostream &os, const Server &server)
{
    os << "----------------------------------------" << std::endl;
    os << BOLD << "Server Information:" << END << std::endl;
    os << "Server: " << server._serverName << std::endl;
    os << "Port: " << server._port << std::endl;
    os << "Server Socket Descriptor: " << server._serverSd << std::endl;
    os << "New Socket Descriptor: " << server._newSd << std::endl;
    os << "Password: " << server._password << std::endl;
    os << "Clients: " << std::endl;
    std::vector<Client> _clients = server.get_clients();
    for (int i = 0; i < (int)_clients.size(); i++)
    {
        Client client = server._clients[i];
        os << "  - " << client.getNickname() << " | " << client.getIp() << ":" << client.getPort() << std::endl;
    }
    os << "----------------------------------------" << std::endl;
    
    return os;
}