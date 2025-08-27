/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 12:28:49 by root              #+#    #+#             */
/*   Updated: 2025/07/06 14:53:51 by root             ###   ########.fr       */
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
#include "../include/Outline.hpp"

std::vector<Message>    check_for_new_messages(Server &server)
{
    std::vector<Message>    _newMessages;
    std::vector<Client>     _clients = server.get_clients();

    for (int i = 0; i < (int)_clients.size(); i++)
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
        std::cout << Outline("Usage: ./ircserv <port> <password>", RED, WHITE, "WARNING");
        std::cout << Outline("Example: ./ircserv 25565 mypassword", CYAN, WHITE, "INFO");
        return (EXIT_FAILURE);
    }

    std::cout << Outline("Welcome to the IRC server", GREEN, WHITE, "INFO");
    std::cout << Outline("Made by MSOLINSK and IDOMAGAL", YELLOW, WHITE, "INFO");
    std::cout << Outline("ShortText", WHITE, RED, "Example of some long text that is longer than main one") << std::endl << std::endl;

    Server server(atoi(argv[1]), argv[2]);
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
            else
            {
                Client &newClient = server.get_clients().back();
                server.send(newClient, ":server 001 " + newClient.getNickname() + " :Welcome to the IRC server");
                server.send(newClient, ":server 002 " + newClient.getNickname() + " :Your host is server, running version 1.0");
                server.send(newClient, ":server 003 " + newClient.getNickname() + " :This server was created today");
                server.send(newClient, ":server 004 " + newClient.getNickname() + " server 1.0 o o");
            }
        }

        std::vector<Client> clients = server.get_clients();
        for (size_t j = 0; j < clients.size(); ++j)
        {
            if (FD_ISSET(clients[j].getFd(), &readfds))
            {
                Message msg = server.recv(clients[j]);
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

        // std::vector<Channel> &channels = server.get_channels();
        // for (size_t i = 0; i < channels.size(); i++)
        // {
        //     // data on existing clients?
        //     std::vector<Client> clients = channels[i].getClients();
        //     for (size_t j = 0; j < clients.size(); ++j)
        //     {
        //         if (FD_ISSET(clients[j].getFd(), &readfds))
        //         {
        //             Message msg = server.recv(clients[j]);
        //             if (!msg.isValid())
        //                 continue;
        //             if (msg.getContent() == SERVER_SHUTDOWN)
        //             {
        //                 std::cout << "\n\n" << server << "\n\n";
        //                 server.disconnect();
        //                 break;
        //             }
        //             std::cout << msg << std::endl;
        //             parse_message(server, msg);
        //         }
        //     }
        // }
    }

    for (int i = 0; i < (int)server.get_channels().size(); i++)
    {
        std::cout << server.get_channels()[i] << std::endl;
    }

    return (EXIT_SUCCESS);
}
