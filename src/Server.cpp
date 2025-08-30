#include "../include/utils.hpp"
#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Message.hpp"
#include "../include/Channel.hpp"
#include "../include/Outline.hpp"
#include "../include/Commands.hpp"
#include <vector>
#include <string>
#include <algorithm>

Server::Server(int port, std::string password) : _port(port), _password(password), _serverName("matrix25565")
{
    // Initialize server socket descriptor
    this->_serverSd = -1;
    this->_newSd = -1;

    // Initialize clients and channels vectors
    this->_clients.clear();
    this->_channels.clear();    
}

Server::~Server()
{
    std::cout << WARNING << "Destroyed SERVER" << std::endl;
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
        std::cerr << ERROR << "Error establishing the server socket" << std::endl;
        exit(0);
    }

    // Set socket to non-blocking
    int flags = fcntl(serverSd, F_GETFL, 0);
    fcntl(serverSd, F_SETFL, flags | O_NONBLOCK);

    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, sizeof(servAddr));
    if(bindStatus < 0)
    {
        std::cerr << ERROR << "Error binding socket to local address" << std::endl;
        exit(0);
    }

    ::listen(serverSd, n_clients);
    std::cout << WARNING << "Listening on port " << _port << " for " << n_clients << " clients" << std::endl;

    this->_serverSd = serverSd;
    return 0;
}

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
        std::cerr << ERROR << "Poll error on server socket" << std::endl;
        return -1;
    }
    if (pfd.revents & POLLIN)
    {
        int newSd = accept(this->_serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
        if (newSd < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                return 0; // No new connection yet
            std::cerr << ERROR << "Error accepting request from client!" << std::endl;
            return -1;
        }

        // Make the new client socket non-blocking
        int flags = fcntl(newSd, F_GETFL, 0);
        if (flags < 0)
        {
            std::cerr << ERROR << "Error getting flags on new socket: " << strerror(errno) << std::endl;
            close(newSd);
            return -1;
        }
        if (fcntl(newSd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            std::cerr << ERROR << "Error setting non-blocking on new socket: " << strerror(errno) << std::endl;
            close(newSd);
            return -1;
        }

        this->_newSd = newSd;
        std::string ip = inet_ntoa(newSockAddr.sin_addr);
        int port = ntohs(newSockAddr.sin_port);
        Client newClient(newSd, ip, port);
    
        std::string response = "Connected with client! | SD: ";
        response = append_number(response, newSd);
        response += " | IP: " + ip + " | PORT: ";
        response = append_number(response, port);
        std::cout << INFO << response << std::endl;
        // std::cout << Outline(response, CYAN, WHITE, "New client") << std::endl;
        newClient.setAuthenticated(false);
        // this->halloy_support(newClient);
        this->_clients.push_back(std::move(newClient));
    }
    return 0;
}

Server *Server::get_self()
{
    return (this);
}

void Server::halloy_support(Client &client, Message &msg)
{
    const std::vector<std::string> line_split = split(msg.getContent(), '\n');
    for (int i = 0; i < (int)line_split.size(); i++)
    {
        std::string command = line_split[i];
        // Skip empty lines
        if (command.empty()) {
            continue;
        }
        // Remove carriage returns if present
        size_t cr_pos = command.find('\r');
        if (cr_pos != std::string::npos) {
            command = command.substr(0, cr_pos);
        }
        
        std::cout << WARNING << "Current init command: " << command << std::endl;
        parse_initial_message(*this->get_self(), client, command);
    }
    
}

int Server::disconnect()
{
    std::cout << WARNING << "Server shutdown command received" << std::endl;
    std::cout << WARNING << "Disconnecting clients from server..." << std::endl;
    // std::vector<Client> _clients = this->get_clients();
    std::vector<Client> &_clients = this->get_clients();
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        try
        {
            this->send(_clients[i], "DISCONNECTED");
            close(_clients[i].getSd());
            std::cout
                << SUCCESS
                << _clients[i].getIp() << ":" << _clients[i].getPort() << " | "
                << _clients[i].getNickname() << " | "
                << _clients[i].getSd() << " | "
                << "DISCONNECTED" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << ERROR << "Error closing client socket: " << e.what() << std::endl;
        }
    }
    try
    {
        close(this->_serverSd);
        std::cout << SUCCESS << "Server socket closed" << std::endl;
        this->_clients.clear();
        this->_serverSd = -1;
        this->_newSd = -1;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
        
    return (0);
}

void    Server::send(Client &client, std::string msg)
{
    if (msg == "DISCONNECTED")
    {
        std::cout << INFO << "Client disconnected" << std::endl;
        return;
    }
    
    // Check if the client socket is valid
    if (client.getSd() < 0) {
        std::cerr << ERROR << "Attempting to send to invalid socket descriptor" << std::endl;
        return;
    }
    
    // Ensure IRC line termination (CRLF)
    if (msg.size() >= 2) {
        if (msg.compare(msg.size() - 2, 2, "\r\n") != 0) {
            if (!msg.empty() && msg.back() == '\n') {
                msg[msg.size() - 1] = '\r';
                msg += '\n';
            } else {
                msg += "\r\n";
            }
        }
    } else {
        if (msg.empty() || msg.back() != '\n') {
            msg += "\r\n";
        } else {
            msg.insert(msg.size() - 1, "\r");
        }
    }
    
    int bytesSent = ::send(client.getSd(), msg.c_str(), msg.length(), 0);
    if (bytesSent < 0)
    {
        std::cerr << ERROR << "Error sending message to client: " << strerror(errno) << std::endl;
        std::cerr << ERROR << "Trying to send: " << msg << std::endl;
    }
}

Message Server::recv(Client &client)
{
    char msg[150];

    // std::cout << "Awaiting client message..." << std::endl;
    memset(&msg, 0, sizeof(msg));   //clear the buffer
    int bytesRead = ::recv(client.getSd(), (char*)&msg, sizeof(msg), 0);
    if (bytesRead == 0 || strcmp(msg, "exit") == 0)
        return Message();
    // else
    // {
    //     if (errno == EWOULDBLOCK || errno == EAGAIN)
    //         ;
    // }
    return (Message(client, client.getNickname(), c_strip(msg), std::time(0), bytesRead));
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

int Server::assign_read_mode(int listen_fd, fd_set &readfds)
{
    int maxfd = listen_fd;

    // std::vector<Client> clients = this->get_clients();
    std::vector<Client> &clients = this->get_clients();
    for (size_t i = 0; i < clients.size(); ++i)
    {
        int fd = clients[i].getFd();
        FD_SET(fd, &readfds);
        maxfd = std::max(maxfd, fd);
    }
    return (maxfd);
}

Channel &Server::access_channel(std::string channelName)
{
    // Accept both "#general" and "general"
    std::string key = channelName;
    if (!key.empty() && key[0] == '#')
        key = key.substr(1);

    for (size_t i = 0; i < this->_channels.size(); ++i)
    {
        const std::string &stored = this->_channels[i].getName();
        if (stored == key)
        {
            std::cout << INFO << "Channel found: <" << stored << ">" << std::endl;
            return (this->_channels[i]);
        }
    }
    std::cout << ERROR << "Channel not found: <" << channelName << ">" << std::endl;
    throw std::runtime_error("Channel not found");
}

void    Server::add_channel(Channel &channel)
{
    this->_channels.push_back(channel);
}

void Server::remove_client(int fd)
{
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i].getSd() == fd)
        {
            for (size_t c = 0; c < _channels.size(); ++c)
                _channels[c].removeClient(_clients[i]);
            std::cout << INFO << "Removing client " << _clients[i].getNickname()
                      << " (" << _clients[i].getIp() << ":" << _clients[i].getPort()
                      << ", fd=" << fd << ")" << std::endl;
            _clients.erase(_clients.begin() + i);
            return;
        }
    }
    std::cout << WARNING << "Requested to remove unknown client fd=" << fd << std::endl;
}

std::string Server::get_serverName() const
{
    return this->_serverName;
}

std::string Server::get_password() const
{
    return this->_password;
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

std::vector<Client>& Server::get_clients()
{
    return this->_clients;
}

const std::vector<Client>& Server::get_clients() const
{
    return this->_clients;
}

std::vector<Channel> &Server::get_channels()
{
    return this->_channels;
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
    // std::vector<Client> _clients = server.get_clients();
    const std::vector<Client> &_clients = server.get_clients();
    for (int i = 0; i < (int)_clients.size(); i++)
    {
        // Client client = server._clients[i];
        const Client &client = server._clients[i];
        os << "  - " << client.getNickname() << " | " << client.getIp() << ":" << client.getPort() << std::endl;
    }
    os << "----------------------------------------" << std::endl;
    
    return os;
}