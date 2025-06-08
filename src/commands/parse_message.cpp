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
    const std::string content = msg.getContent();
    const std::string command = content.substr(0, content.find(' '));
    const std::string args = content.substr(content.find(' ') + 1);
    const std::vector<std::string> arg_vector = split(args, ' ');

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

        const std::string& channelName = arg_vector[0];
        Client &client = msg.getSender();
        const std::string currentChannelName = client.getCurrentChannel();


        std::cout << "\n\n" << WARNING << "Current channel: " << currentChannelName << std::endl;
        std::cout << WARNING << "Joining channel: " << channelName << std::endl;

        try {
            // If client is already in a channel, remove them from it
            if (!currentChannelName.empty() && currentChannelName != channelName) {
                Channel &oldChannel = server.access_channel(currentChannelName);
                oldChannel.removeClient(client);
                std::cout << INFO << "Client " << client.getNickname() << " left channel " << currentChannelName << std::endl;
            }

            // Join the new channel
            Channel &newChannel = server.access_channel(channelName);
            // Check if client is already in the target channel
            bool alreadyInChannel = false;
            std::vector<Client> clientsInNewChannel = newChannel.getClients();
            for (size_t i = 0; i < clientsInNewChannel.size(); ++i) {
                if (clientsInNewChannel[i].getSd() == client.getSd()) {
                    alreadyInChannel = true;
                    break;
                }
            }

            if (!alreadyInChannel) {
                newChannel.addClient(client);
                std::cout << ERROR << "Before entering function: " << channelName << std::endl;
                client.setCurrentChannel(channelName);
                std::cout << INFO << "Client " << client.getNickname() << " joined channel " << channelName << std::endl;
                
                // You might want to send a confirmation message to the client here
                // server.send(client, "You have joined " + channelName);
                // And notify other clients in the channel
                // newChannel.broadcastMessage(client.getNickname() + " has joined the channel.", client);
            } else {
                std::cout << INFO << "Client " << client.getNickname() << " is already in channel " << channelName << std::endl;
            }

        } catch (const std::runtime_error& e) {
            std::cerr << ERROR << "Error joining channel: " << e.what() << std::endl;
            // Optionally, send an error message back to the client
            // server.send(client, "ERROR :Could not join channel " + channelName + ". " + e.what());
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
