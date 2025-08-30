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
#include "../../include/Outline.hpp"


void parse_message(Server &server, Message &msg)
{
    const std::string content = msg.getContent();
    const std::string command = content.substr(0, content.find(' '));
    const std::string args = content.substr(content.find(' ') + 1);
    const std::vector<std::string> arg_vector = split(args, ' ');

    std::cout << "ARG_VECTOR[0]: " << arg_vector[0] << std::endl;

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
        Channel &channel = server.access_channel(msg.getSender().getCurrentChannel());

        if (channel.check_password(arg_vector[0]))
        {
            msg.getSender().setAuthenticated(true);
            std::cout << SUCCESS << "Password accepted for client: " << msg.getSender().getFd() << std::endl;
        }
        else
        {
            std::cerr << ERROR << "Incorrect password for client: " << msg.getSender().getNickname() << std::endl;
            close(msg.getSender().getSd());
            return;
        }
    }
    else if (command == "NICK")
    {
        if (arg_vector.empty() || arg_vector[0].empty()) {
            std::cerr << ERROR << "NICK command requires a valid nickname" << std::endl;
            return;
        }

        Client &client = msg.getSender();
        const std::string newNickname = arg_vector[0];

        // Check if the nickname is already taken
        for (int i = 0; i < (int)server.get_clients().size(); ++i)
        {
            Client &c = server.get_clients()[i];
            if (c.getNickname() == newNickname) {
                std::cerr << ERROR << "Nickname already taken: " << newNickname << std::endl;
                server.send(client, "ERROR: Nickname already taken");
                return;
            }
        }

        client.setNickname(newNickname);
        std::cout << INFO << "Client nickname set to: " << client.getNickname() << std::endl;

        // Optionally, send a confirmation message back to the client
        // server.send(client, "NICK :You are now known as " + newNickname);
    }
    else if (command == "USER")
    {
        if (arg_vector.size() < 2 || arg_vector[0].empty() || arg_vector[1].empty()) {
            std::cerr << ERROR << "USER command requires a valid username and real name" << std::endl;
            return;
        }
        Client &client = msg.getSender();
        client.setUsername(arg_vector[0]);
        client.setRealName(arg_vector[1]);
        
        std::string response = ":server 001 " + client.getNickname() + " :Welcome to the IRC server, " + client.getNickname() + "!";
        response += " You are now known as " + client.getNickname() + " (" + client.getUsername() + ") - " + client.getRealName() + "\n";
        server.send(client, response);
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
            std::vector<Client*> &clientsInNewChannel = newChannel.getClients();
            for (size_t i = 0; i < clientsInNewChannel.size(); ++i) {
                if (clientsInNewChannel[i] && clientsInNewChannel[i]->getSd() == client.getSd()) {
                    alreadyInChannel = true;
                    break;
                }
            }

            if (!alreadyInChannel) {
                newChannel.addClient(client);
                std::cout << ERROR << "Before entering function: " << channelName << std::endl;
                client.setCurrentChannel(channelName);
                std::cout << INFO << "Client " << client.getNickname() << " joined channel " << channelName << std::endl;

                // Send server responses
                server.send(client, ":server 332 " + client.getNickname() + " " + channelName + " :" + newChannel.getTopic());
                server.send(client, ":server 333 " + client.getNickname() + " " + channelName + " " + client.getNickname() + " " + get_current_timestamp());

                // List users in the channel
                std::string userList = client.getNickname();
                std::vector<Client*> &clientsInChannel = newChannel.getClients();
                for (size_t i = 0; i < clientsInChannel.size(); ++i) {
                    if (clientsInChannel[i] && clientsInChannel[i]->getNickname() != client.getNickname()) {
                        userList += " " + clientsInChannel[i]->getNickname();
                    }
                }
                server.send(client, ":server 353 " + client.getNickname() + " = " + channelName + " :" + userList);
                server.send(client, ":server 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list");
            } else {
                std::cout << INFO << "Client " << client.getNickname() << " is already in channel " << channelName << std::endl;
                server.send(client, ":server 331 " + client.getNickname() + " #" + channelName + " :You are already in this channel\n");
            }

        } catch (const std::runtime_error& e) {
            std::cerr << ERROR << "Error joining channel: " << e.what() << std::endl;
            // Optionally, send an error message back to the client
            // server.send(client, "ERROR :Could not join channel " + channelName + ". " + e.what());
        }
    }
    else if (command == "LIST")
    {
        Client &client = msg.getSender();
        server.send(client, ":server 321 " + client.getNickname() + " Channel :Users Name");

        std::vector<Channel> &channels = server.get_channels();
        std::cout << INFO << "Available channels:" << std::endl;
        for (size_t i = 0; i < channels.size(); ++i)
        {
            int clientCount = channels[i].getClients().size();
            if (channels[i].getName().empty())
            {
                std::string response = append_number(" <no-name> ", clientCount);
                response += " :" + channels[i].getTopic();
                server.send(client, ":server 322 " + client.getNickname() + response);
            }
            else
            {
                // Do NOT add another '#', name already includes it.
                std::string response = " " + channels[i].getName() + " ";
                response = append_number(response, clientCount);
                response += " :" + channels[i].getTopic();
                server.send(client, ":server 322 " + client.getNickname() + response);
            }
            std::cout << channels[i].getName() << " | Clients: " << clientCount << " | Topic: " << channels[i].getTopic() << std::endl;
        }
        server.send(client, ":server 323 " + client.getNickname() + " :End of /LIST");
    }
    else if (command == "CAP")
    {
        if (arg_vector.empty() || arg_vector[0].empty())
        {
            std::cerr << ERROR << "CAP command requires a valid argument" << std::endl;
            return;
        }
        Client &client = msg.getSender();
        std::string capCommand = arg_vector[0];

        if (capCommand == "LS")
        {
            std::string response = ":server  CAP * LS :\r\n";
            server.send(client, response);
        }
        else if (capCommand == "REQ")
        {
            if (arg_vector.size() < 2 || arg_vector[1].empty())
            {
                std::cerr << ERROR << "CAP REQ command requires a valid capability" << std::endl;
                return;
            }
            server.send(client, ":server  CAP * ACK " + arg_vector[1]);
        }
        else if (capCommand == "END")
            std::cout << "Client negotiation ended successfully!" << std::endl;
        else
        {
            std::cerr << ERROR << "Unknown CAP command: " << capCommand << std::endl;
            server.send(client, ":server  CAP * NAK :Unknown command");
        }
    }
    else if (command == "KICK")
    {
        if (arg_vector.size() < 2 || arg_vector[0].empty() || arg_vector[1].empty())
        {
            std::cerr << ERROR << "KICK command requires a valid channel and user" << std::endl;
            return;
        }
        
        Channel &channel = server.access_channel(arg_vector[0]);
        Client &client = get_client_from_channel_by_name(channel, arg_vector[1]);
        channel.removeClient(client);
        std::cout << WARNING << "Kicking user: " << arg_vector[1] << " from channel " << arg_vector[0] << std::endl;
        std::string response = ":server  KICK " + arg_vector[0] + " " + arg_vector[1] + " :You have been kicked from the channel\n";
        server.send(client, response);
    }
    else if (command == "PING")
    {
        std::cout << INFO << "Received PING from " << msg.getSender().getNickname() << std::endl;
        std::string response = "PONG :" + arg_vector[0] + "\r\n";
        server.send(msg.getSender(), response);
    }
    else if (command == "QUIT")
    {
        Client &client = msg.getSender();
        std::cout << WARNING << "Client " << client.getNickname() << " is disconnecting." << std::endl;
        server.send(client, ":server ERROR :Closing Link: " + client.getNickname() + "\r\n");
        close(client.getSd());
        // Optionally, remove the client from all channels
        std::vector<Channel> &channels = server.get_channels();
        for (size_t i = 0; i < channels.size(); ++i) {
            channels[i].removeClient(client);
        }
    }
    else
    {
        std::cerr << ERROR << "Unknown command: " << command << std::endl;
        server.send(msg.getSender(), "ERROR :Unknown command: " + command);
    }
}

void    parse_initial_message(Server &server, Client &client, std::string cmd)
{
    const std::vector<std::string> arg_vector = split(cmd, ' ');

    if (arg_vector[0] == "PASS")
    {
        // Channel &channel = server.access_channel(msg.getSender().getCurrentChannel());
        // std::cout << Outline("Server password: pass | User provided password: " + arg_vector[0], RED, RED, "");
        // std::cout << "Password: " << arg_vector[0] << std::endl;
        if (server.check_password(arg_vector[1]))
            std::cout << SUCCESS << "Password accepted for client:\n" << client << std::endl;
        else
        {
            std::cerr << ERROR << "Incorrect password for client:\n" << client << std::endl;
            server.send(client, "[*" + server.get_serverName() + "*] " + "Password incorrect.");
            return;
        }
    }
    else if (arg_vector[0] == "CAP")
    {
        if (arg_vector.empty() || arg_vector[1].empty())
        {
            std::cerr << ERROR << "CAP command requires a valid argument" << std::endl;
            return;
        }
        if (arg_vector[1] == "LS")
        {
            server.send(client, ":server  CAP * LS :\r\n");
        }
        else if (arg_vector[1] == "END")
        {
            std::cout << SUCCESS << "Client authenticated: " << client.getNickname() << std::endl;
            client.setAuthenticated(true);
            server.send(client, ":" + server.get_serverName() + " CAP * END");
        }
        else if (arg_vector[1] == "REQ")
        {
            if (arg_vector.size() < 2 || arg_vector[1].empty())
            {
                std::cerr << ERROR << "CAP REQ command requires a valid capability" << std::endl;
                return;
            }
            server.send(client, ":server  CAP * ACK " + arg_vector[2]);
        }
        else
        {
            std::cerr << ERROR << "Unknown CAP command: " << arg_vector[1] << std::endl;
            server.send(client, ":server  CAP * NAK :Unknown command");
        }
    }
    else if (arg_vector[0] == "NICK")
    {
        if (arg_vector.empty() || arg_vector[1].empty()) {
            std::cerr << ERROR << "NICK command requires a valid nickname" << std::endl;
            return;
        }
        // Check if the nickname is already taken
        for (int i = 0; i < (int)server.get_clients().size(); ++i)
        {
            Client &c = server.get_clients()[i];
            if (c.getNickname() == arg_vector[1]) {
                std::cerr << ERROR << "Nickname already taken: " << arg_vector[1] << std::endl;
                server.send(client, "ERROR: Nickname already taken");
                return;
            }
        }

        client.setNickname(arg_vector[1]);
        std::cout << INFO << "Client nickname set to: " << client.getNickname() << std::endl;

        // Optionally, send a confirmation message back to the client
        // server.send(client, "NICK :You are now known as " + newNickname);
    }
    else if (arg_vector[0] == "USER")
    {
        // Check that we have at least 4 parameters for USER command
        // USER <username> <mode> <unused> :<realname>
        if (arg_vector.size() < 4) {
            std::cerr << ERROR << "USER command requires username, mode, unused, and realname" << std::endl;
            return;
        }
        std::string username = arg_vector[1];
        std::string realname;
        
        // Handle the realname parameter which may contain spaces
        // If it starts with a colon, it's typically the remainder of the command
        if (arg_vector.size() > 3) {
            if (arg_vector[3].at(0) == ':') {
                // Remove the colon and use the rest as realname
                realname = arg_vector[3].substr(1);
            } else {
                // Use the fourth parameter as realname
                realname = arg_vector[3];
            }
        }
        
        client.setUsername(arg_vector[1]);
        client.setRealName(realname);
        
        // Send a properly formatted welcome message
        std::string response = ":server 001 " + client.getNickname() + " :Welcome to the IRC server, " + client.getNickname() + "!\r\n";
        response += ":server 002 " + client.getNickname() + " :Your host is server, running version 1.0\r\n";
        response += ":server 003 " + client.getNickname() + " :This server was created today\r\n";
        response += ":server 004 " + client.getNickname() + " server 1.0 o o\r\n";
        server.send(client, response);\
        std::cout << client << std::endl;
    }
    else
    {
        server.send(client, ":server ERROR: Expected different initial info msg");
    }
}