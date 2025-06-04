/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 12:28:49 by root              #+#    #+#             */
/*   Updated: 2025/06/04 14:50:01 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include "../include/Server.hpp"
#include "../include/utils.hpp"
#include "../include/Message.hpp"
#include "../include/Channel.hpp"
#include "../include/Commands.hpp"
#include "../include/Headers.hpp"

std::vector<Message>    check_for_new_messages(Server &server)
{
    std::vector<Message>    _newMessages;
    std::vector<Client>     _clients = server.get_clients();

    for(int i = 0; i < (int)_clients.size(); i++)
    {
        Message msg = server.recv(_clients[i]);
        std::cout << "recv non-blocking" << std::endl;
        if (msg.isValid())  
            _newMessages.push_back(msg);
    }
    return (_newMessages);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << WARNING << "Usage: ./ircserv <port> <password>" << std::endl;
        std::cout << INFO << "Example: ./ircserv 25566 mypassword" << std::endl;
        return (EXIT_FAILURE);
    }

    Server server(atoi(argv[1]));
    server.start_listening(1);

    // 1) grab listen fd & make it non‐blocking
    int listen_fd = server.getListenFd();
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);

    Channel channel("general", "General channel for chatting", argv[2]);
    Channel channel2("random", "Random channel for chatting", argv[2]);
    Channel channel3("private", "Private channel for chatting", argv[2]);
    
    server.add_channel(channel);
    server.add_channel(channel2);
    server.add_channel(channel3);

    // 2) multiplex accept + recv
    while (true)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        int maxfd = listen_fd;

        maxfd = server.assign_read_mode(listen_fd, readfds);
        if (wait_for_activity(maxfd, readfds) == -1)
            break;

        // new connection?
        if (FD_ISSET(listen_fd, &readfds))
        {
            if (server.accept_new_client() < 0)
                std::cerr << ERROR << "Error accepting new client" << std::endl;
        }

        std::vector<Channel> channels = server.get_channels();
        for (size_t i = 0; i < channels.size(); i++)
        {
            // data on existing clients?
            std::vector<Client> clients = channels[i].getClients();
            for (size_t i = 0; i < clients.size(); ++i)
            {
                if (FD_ISSET(clients[i].getFd(), &readfds))
                {
                    Message msg = server.recv(clients[i]);
                    if (!msg.isValid())
                        continue;
                    if (msg.getContent() == SERVER_SHUTDOWN)
                    {
                        std::cout << "\n\n" << server << "\n\n";
                        server.disconnect();
                        break;
                    }
                    std::cout << msg << std::endl;
                    parse_message(server, msg);
                }
            }
        }
    }

    for (int i = 0; i < (int)server.get_channels().size(); i++)
    {
        std::cout << server.get_channels()[i] << std::endl;
    }

    return (EXIT_SUCCESS);
}
