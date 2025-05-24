/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_message.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 14:58:46 by root              #+#    #+#             */
/*   Updated: 2025/05/24 22:39:21 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include "../../include/Message.hpp"
#include "../../include/Commands.hpp"
#include "../../include/Server.hpp"
#include "../../include/utils.hpp"
#include "../../include/Channel.hpp"
#include "../../include/Headers.hpp"
#include "../../include/Client.hpp"


void parse_message(Server &server, Message &msg)
{
    std::string content = msg.getContent();
    std::string command = content.substr(0, content.find(" "));
    std::string args = content.substr(content.find(" ") + 1);
    std::vector<std::string> arg_vector = split(args, ' ');

    if (arg_vector.empty())
    {
        std::cerr << ERROR << "No arguments provided" << std::endl;
        return;
    }
    if (msg.isEmpty())
    {
        std::cerr << ERROR << "Empty message received" << std::endl;
        return;
    }

    if (command == "PASS")
    {
        std::cout << WARNING << "Password: " << arg_vector[0] << std::endl;
    }
    else if (command == "JOIN")
    {
        if (arg_vector.empty() || arg_vector[0].empty()) {
            std::cerr << ERROR << "JOIN command requires a valid channel name" << std::endl;
            return;
        }

        try
        {
            // Get the client's current channel
            std::string currentChannel = msg.getSender().getCurrentChannel();

            // If the client is already in a channel, remove them from it
            if (!currentChannel.empty()) {
                server.access_channel(currentChannel).removeClient(msg.getSender());
                std::cout << INFO << "Client " << msg.getSender().getNickname() 
                          << " removed from channel " << currentChannel << std::endl;
            }

            // Add the client to the new channel
            Channel &newChannel = server.access_channel(arg_vector[0]);
            std::vector<Client> clients = newChannel.getClients();
            bool alreadyInChannel = false;
            for (size_t i = 0; i < clients.size(); ++i) {
                if (clients[i] == msg.getSender()) {
                    alreadyInChannel = true;
                    break;
                }
            }
            if (alreadyInChannel) {
                std::cerr << WARNING << "Client " << msg.getSender().getNickname() 
                          << " is already in channel " << arg_vector[0] << std::endl;
                return;
            }
            newChannel.addClient(msg.getSender());

            // Update the client's current channel
            msg.getSender().setCurrentChannel(arg_vector[0]);

            std::cout << INFO << "Client " << msg.getSender().getNickname() 
                      << " joined channel " << arg_vector[0] << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << ERROR << "Error joining channel: " << e.what() << std::endl;
            return;
        }
    }
    else if (command == "LIST")
    {
        std::vector<Channel> channels = server.get_channels();
        std::cout << INFO << "Available channels:" << std::endl;
        for (size_t i = 0; i < channels.size(); ++i)
        {
            int clientCount = channels[i].getClients().size();
            std::cout << channels[i].getName() << " | Clients: " << clientCount << " | Topic: " << channels[i].getTopic() << std::endl;
        }
    }
    else
    {
        ft_log("Unknown command", "Command: " + command, 2);
    }
}
