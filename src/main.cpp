/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 12:28:49 by root              #+#    #+#             */
/*   Updated: 2025/04/18 22:54:17 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include "../include/Server.hpp"
#include "../include/utils.hpp"
#include "../include/Message.hpp"
#include "../include/Channel.hpp"
#include "../include/Commands.hpp"

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
        ft_log("Usage: ./ircserv [port] [password]", "", 3);
        return -1;
    }

    Server server(atoi(argv[1]));
    server.start_listening(1);

    // 1) grab listen fd & make it non‐blocking
    int listen_fd = server.getListenFd();
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);

    std::vector<Channel> channels;
    Channel channel("general", "General channel for chatting", argv[2]);

    // 2) multiplex accept + recv
    while (server.get_clients().size() < 3)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        int maxfd = listen_fd;

        // add each client socket
        std::vector<Client> clients = server.get_clients();
        for (size_t i = 0; i < clients.size(); ++i)
        {
            int fd = clients[i].getFd();
            FD_SET(fd, &readfds);
            maxfd = std::max(maxfd, fd);
        }

        // wait for activity
        int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            ft_log("select() error", "", 2);
            break;
        }

        // new connection?
        if (FD_ISSET(listen_fd, &readfds))
        {
            if (server.accept_new_client() < 0)
            {
                ft_log("Error accepting new client", "", 2);
            }
        }

        // data on existing clients?
        for (size_t i = 0; i < clients.size(); ++i)
        {
            int fd = clients[i].getFd();
            if (FD_ISSET(fd, &readfds))
            {
                Message msg = server.recv(clients[i]);
                if (msg.isValid())
                    std::cout << msg << std::endl;
            }
        }
    }

    std::cout << server << std::endl;
    ft_log("Shutting down server", "", 0);
    server.disconnect();

    std::cout << std::endl << std::endl;
    std::cout << channel << std::endl;
    return 0;
}


// int main(int argc, char *argv[])
// {
//     if (argc != 3)
//     {
//         ft_log("Usage: ./ircserv [port] [password]", "", 3);
//         return (-1);
//     }

//     Server server = Server(atoi(argv[1]));
//     server.start_listening(1);

// 	std::vector<Channel> channels;
//     Channel channel = Channel("general", "General channel for chatting", argv[2]);

//     while (server.get_clients().size() < 3)
//     {
//         if (server.accept_new_client() == -1)
//         {
//             ft_log("Error accepting new client", "", 2);
//             break;
//         }
//         std::vector<Message>    _newMessages = check_for_new_messages(server);
//         for (int i = 0; i < (int)_newMessages.size(); i++)
//             std::cout << _newMessages[i] << std::endl;
//         // std::vector<Client> clients = server.get_clients();
//     }
    
//     std::cout << server << std::endl;
//     // Message msg = server.recv();
//     // while (msg.isValid())
//     // {
//     //     if (msg.isEmpty())
//     //     {
//     //         ft_log("Client disconnected", "", 2);
//     //         break;
//     //     }
//     //     std::cout << msg << std::endl;
//     //     channel.addMessage(msg);
//     //     parse_message(msg);
//     //     msg = server.recv();
//     // }
//     ft_log("Shutting down server", "", 0);
//     server.disconnect();

//     std::cout << std::endl << std::endl;
//     std::cout << channel << std::endl;

//     return 0;
// }