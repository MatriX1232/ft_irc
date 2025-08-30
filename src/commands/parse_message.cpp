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


// Small helpers for channel name normalization
static inline std::string strip_hash(const std::string &name) {
    return (!name.empty() && name[0] == '#') ? name.substr(1) : name;
}
static inline std::string ensure_hash(const std::string &name) {
    return (!name.empty() && name[0] == '#') ? name : "#" + name;
}


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
        // Removed sending 001 here; registration numerics are sent on CAP END.
    }
    else if (command == "JOIN")
    {
        if (arg_vector.empty() || arg_vector[0].empty()) {
            std::cerr << ERROR << "JOIN command requires a valid channel name" << std::endl;
            return;
        }

        const std::string rawName = arg_vector[0];
        const std::string chan = strip_hash(rawName);
        Client &client = msg.getSender();
        const std::string currentChannelName = client.getCurrentChannel();


        std::cout << "\n\n" << WARNING << "Current channel: " << currentChannelName << std::endl;
        std::cout << WARNING << "Joining channel: " << chan << std::endl;

        try {
            // If client is already in a channel, remove them from it
            if (!currentChannelName.empty() && currentChannelName != chan) {
                Channel &oldChannel = server.access_channel(currentChannelName);
                oldChannel.removeClient(client);
                std::cout << INFO << "Client " << client.getNickname() << " left channel " << currentChannelName << std::endl;
            }

            // Join the new channel
            Channel &newChannel = server.access_channel(chan);
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
                client.setCurrentChannel(chan);
                std::cout << INFO << "Client " << client.getNickname() << " joined channel " << chan << std::endl;

                // 1) Broadcast JOIN to all members (Halloy needs this to enable input)
                const std::string vis = ensure_hash(chan);
                const std::string prefix = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getIp();
                for (size_t i = 0; i < clientsInNewChannel.size(); ++i) {
                    if (clientsInNewChannel[i]) {
                        server.send(*clientsInNewChannel[i], prefix + " JOIN " + vis);
                    }
                }

                // 2) Topic (+ who/time) numerics
                server.send(client, ":server 332 " + client.getNickname() + " " + vis + " :" + newChannel.getTopic());
                server.send(client, ":server 333 " + client.getNickname() + " " + vis + " " + client.getNickname() + " " + get_current_timestamp());

                // 3) Names + end of names
                std::string userList = client.getNickname();
                std::vector<Client*> &clientsInChannel = newChannel.getClients();
                for (size_t i = 0; i < clientsInChannel.size(); ++i) {
                    if (clientsInChannel[i] && clientsInChannel[i]->getNickname() != client.getNickname()) {
                        userList += " " + clientsInChannel[i]->getNickname();
                    }
                }
                server.send(client, ":server 353 " + client.getNickname() + " = " + vis + " :" + userList);
                server.send(client, ":server 366 " + client.getNickname() + " " + vis + " :End of /NAMES list");
            } else {
                server.send(client, ":server 331 " + client.getNickname() + " " + ensure_hash(chan) + " :You are already in this channel");
            }
        } catch (const std::runtime_error& e) {
            std::cerr << ERROR << "Error joining channel: " << e.what() << std::endl;
        }
    }
    else if (command == "PRIVMSG")
    {
        if (args.empty()) {
            server.send(msg.getSender(), ":server 412 " + msg.getSender().getNickname() + " :No text to send");
            return;
        }

        // Parse: PRIVMSG <target> :<message>
        // Extract target
        size_t sp = args.find(' ');
        if (sp == std::string::npos) {
            server.send(msg.getSender(), ":server 411 " + msg.getSender().getNickname() + " :No recipient given (PRIVMSG)");
            return;
        }
        std::string target = args.substr(0, sp);
        // Extract message after " :"
        std::string text;
        size_t col = args.find(" :");
        if (col != std::string::npos) {
            text = args.substr(col + 2);
        } else {
            // Fallback: rest after first space
            text = args.substr(sp + 1);
        }
        if (text.empty()) {
            server.send(msg.getSender(), ":server 412 " + msg.getSender().getNickname() + " :No text to send");
            return;
        }

        const std::string prefix = ":" + msg.getSender().getNickname() + "!" + msg.getSender().getUsername() + "@" + msg.getSender().getIp();

        if (!target.empty() && target[0] == '#') {
            // Channel message
            const std::string chan = strip_hash(target);
            try {
                Channel &channel = server.access_channel(chan);
                std::vector<Client*> &clientsInChannel = channel.getClients();
                const std::string vis = ensure_hash(chan);
                for (size_t i = 0; i < clientsInChannel.size(); ++i) {
                    if (clientsInChannel[i]) {
                        // Deliver to everyone including sender (usual IRC behavior)
                        server.send(*clientsInChannel[i], prefix + " PRIVMSG " + vis + " :" + text);
                    }
                }
            } catch (const std::runtime_error&) {
                server.send(msg.getSender(), ":server 403 " + msg.getSender().getNickname() + " " + target + " :No such channel");
            }
        } else {
            // User-to-user message
            bool delivered = false;
            std::vector<Client> &clients = server.get_clients();
            for (size_t i = 0; i < clients.size(); ++i) {
                if (clients[i].getNickname() == target) {
                    server.send(clients[i], prefix + " PRIVMSG " + target + " :" + text);
                    // Echo back to sender so UI displays it immediately
                    server.send(msg.getSender(), prefix + " PRIVMSG " + target + " :" + text);
                    delivered = true;
                    break;
                }
            }
            if (!delivered) {
                server.send(msg.getSender(), ":server 401 " + msg.getSender().getNickname() + " " + target + " :No such nick");
            }
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
            std::string response = " " + ensure_hash(channels[i].getName()) + " ";
            response = append_number(response, clientCount);
            response += " :" + channels[i].getTopic();
            server.send(client, ":server 322 " + client.getNickname() + response);
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
        
        Channel &channel = server.access_channel(strip_hash(arg_vector[0]));
        Client &client = get_client_from_channel_by_name(channel, arg_vector[1]);
        channel.removeClient(client);
        std::cout << WARNING << "Kicking user: " << arg_vector[1] << " from channel " << arg_vector[0] << std::endl;
        std::string response = ":server  KICK " + ensure_hash(strip_hash(arg_vector[0])) + " " + arg_vector[1] + " :You have been kicked from the channel\n";
        server.send(client, response);
    }
    else if (command == "PING")
    {
        std::cout << INFO << "Received PING from " << msg.getSender().getNickname() << std::endl;
        std::string response = "PONG :" + arg_vector[0] + "\r\n";
        server.send(msg.getSender(), response);
    }
    else if (command == "WHO")
    {
        if (arg_vector.empty() || arg_vector[0].empty()) {
            std::cerr << ERROR << "WHO command requires a valid channel name" << std::endl;
            return;
        }

        const std::string chan = strip_hash(arg_vector[0]);
        Client &client = msg.getSender();

        try
        {
            Channel &channel = server.access_channel(chan);
            if (channel.getClients().empty())
                return (server.send(client, ":server 315 " + client.getNickname() + " " + ensure_hash(chan) + " :End of /WHO list\r\n"));
            std::vector<Client*> &clientsInChannel = channel.getClients();
            for (size_t i = 0; i < clientsInChannel.size(); ++i) {
                if (clientsInChannel[i])
                {
                    std::string response = ":server 352 " + client.getNickname() + " " + ensure_hash(chan) + " " +
                                           clientsInChannel[i]->getUsername() + " " +
                                           clientsInChannel[i]->getIp() + " server " +
                                           clientsInChannel[i]->getNickname() + " H :0 " +
                                           clientsInChannel[i]->getRealName() + "\r\n";
                    server.send(client, response);
                }
            }
            server.send(client, ":server 315 " + client.getNickname() + " " + ensure_hash(chan) + " :End of /WHO list\r\n");
        } catch (const std::runtime_error& e) {
            std::cerr << ERROR << "Error processing WHO command: " << e.what() << std::endl;
            server.send(client, ":server 401 " + client.getNickname() + " " + ensure_hash(chan) + " :No such channel\r\n");
        }
    }
    else if (command == "QUIT")
    {
        Client &client = msg.getSender();
        std::cout << WARNING << "Client " << client.getNickname() << " is disconnecting." << std::endl;
        close(client.getSd());
        server.remove_client(client.getSd());
        return;
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
        if (arg_vector.size() < 2) {
            server.send(client, ":server 464 * :Password required");
            return;
        }
        if (server.check_password(arg_vector[1]))
            std::cout << SUCCESS << "Password accepted for client:\n" << client << std::endl;
        else
        {
            std::cerr << ERROR << "Incorrect password for client:\n" << client << std::endl;
            server.send(client, ":" + server.get_serverName() + " NOTICE * :Password incorrect");
            return;
        }
    }
    else if (arg_vector[0] == "CAP")
    {
        if (arg_vector.size() < 2) {
            server.send(client, ":" + server.get_serverName() + " CAP * NAK :Missing subcommand");
            return;
        }

        const std::string sub = arg_vector[1];

        if (sub == "LS")
        {
            // Reply to CAP LS with an (empty) capabilities list for now.
            server.send(client, ":" + server.get_serverName() + " CAP * LS :");
        }
        else if (sub == "REQ")
        {
            // Typical client sends: CAP REQ :cap1 cap2
            // Our split() keeps the colon on the first capability token; reassemble trailing list:
            if (arg_vector.size() < 3) {
                server.send(client, ":" + server.get_serverName() + " CAP * NAK :");
                return;
            }
            std::string caps;
            for (size_t i = 2; i < arg_vector.size(); ++i) {
                if (!caps.empty()) caps += " ";
                caps += arg_vector[i];
            }
            // ACK what was requested (or validate if you want)
            server.send(client, ":" + server.get_serverName() + " CAP * ACK " + (caps[0] == ':' ? caps : ":" + caps));
        }
        else if (sub == "END")
        {
            // Require USER (and NICK) to be set before accepting CAP END.
            const bool hasNick = !client.getNickname().empty();
            const bool hasUser = !client.getUsername().empty();
            if (!hasNick || !hasUser)
            {
                server.send(client, ":" + server.get_serverName() + " NOTICE * :Send CAP END after NICK and USER");
                return;
            }

            std::cout << SUCCESS << "Client authenticated: " << client.getNickname() << std::endl;
            client.setAuthenticated(true);

            const std::string nick = client.getNickname().empty() ? "*" : client.getNickname();

            // 001/002/004 welcome numerics
            server.send(client, ":" + server.get_serverName() + " 001 " + nick + " :Welcome to the IRC server, " + nick + "!");
            server.send(client, ":" + server.get_serverName() + " 002 " + nick + " :Your host is " + server.get_serverName() + ", running version 1.0");
            server.send(client, ":" + server.get_serverName() + " 004 " + nick + " " + server.get_serverName() + " 1.0 o o");
            // MOTD sequence to signal end of registration and enable auto-join
            server.send(client, ":" + server.get_serverName() + " 375 " + nick + " :- " + server.get_serverName() + " Message of the day -");
            server.send(client, ":" + server.get_serverName() + " 372 " + nick + " :- Welcome to " + server.get_serverName());
            server.send(client, ":" + server.get_serverName() + " 372 " + nick + " :- Type /join #general to start chatting.");
            server.send(client, ":" + server.get_serverName() + " 376 " + nick + " :End of /MOTD command");
        }
        else
        {
            server.send(client, ":" + server.get_serverName() + " CAP * NAK :Unknown subcommand");
        }
    }
    else if (arg_vector[0] == "NICK")
    {
        if (arg_vector.size() < 2 || arg_vector[1].empty()) {
            std::cerr << ERROR << "NICK command requires a valid nickname" << std::endl;
            return;
        }
        for (int i = 0; i < (int)server.get_clients().size(); ++i)
        {
            Client &c = server.get_clients()[i];
            if (c.getNickname() == arg_vector[1]) {
                std::cerr << ERROR << "Nickname already taken: " << arg_vector[1] << std::endl;
                server.send(client, "ERROR :Nickname already taken");
                return;
            }
        }
        client.setNickname(arg_vector[1]);
        std::cout << INFO << "Client nickname set to: " << client.getNickname() << std::endl;
    }
    else if (arg_vector[0] == "USER")
    {
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
    }
    else
    {
        server.send(client, ":server ERROR: Expected different initial info msg");
    }
}