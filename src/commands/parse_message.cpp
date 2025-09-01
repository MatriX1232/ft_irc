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

// Helper: find client by nickname
static Client* find_client_by_nick(Server &server, const std::string &nick) {
    std::vector<Client> &clients = server.get_clients();
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].getNickname() == nick)
            return &clients[i];
    }
    return NULL;
}

// Helper: find client by fd
static Client* find_client_by_fd(Server &server, int fd) {
    std::vector<Client> &clients = server.get_clients();
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].getSd() == fd)
            return &clients[i];
    }
    return NULL;
}

// New: command id mapping for switch
enum Cmd {
    CMD_UNKNOWN = 0,
    CMD_PASS,
    CMD_NICK,
    CMD_USER,
    CMD_JOIN,
    CMD_PRIVMSG,
    CMD_LIST,
    CMD_CAP,
    CMD_KICK,
    CMD_INVITE,
    CMD_TOPIC,
    CMD_MODE,
    CMD_PING,
    CMD_WHO,
    CMD_QUIT
};
static inline Cmd command_id(const std::string &s) {
    if (s == "PASS") return CMD_PASS;
    if (s == "NICK") return CMD_NICK;
    if (s == "USER") return CMD_USER;
    if (s == "JOIN") return CMD_JOIN;
    if (s == "PRIVMSG") return CMD_PRIVMSG;
    if (s == "LIST") return CMD_LIST;
    if (s == "CAP") return CMD_CAP;
    if (s == "KICK") return CMD_KICK;
    if (s == "INVITE") return CMD_INVITE;
    if (s == "TOPIC") return CMD_TOPIC;
    if (s == "MODE") return CMD_MODE;
    if (s == "PING") return CMD_PING;
    if (s == "WHO") return CMD_WHO;
    if (s == "QUIT") return CMD_QUIT;
    return CMD_UNKNOWN;
}

void parse_message(Server &server, Message &msg)
{
    const std::string content = msg.getContent();

    // Robust command/args extraction
    size_t sp = content.find(' ');
    const std::string command = (sp == std::string::npos) ? content : content.substr(0, sp);
    const std::string args = (sp == std::string::npos) ? "" : content.substr(sp + 1);
    const std::vector<std::string> arg_vector = args.empty() ? std::vector<std::string>() : split(args, ' ');

    if (msg.isEmpty())
    {
        std::cerr << ERROR << "Empty message received" << std::endl;
        return;
    }

    switch (command_id(command))
    {
        case CMD_PASS:
        {
            if (arg_vector.empty() || arg_vector[0].empty()) {
                // server-style numeric
                server.send(msg.getSender(), ":server 464 * :Password required");
                return;
            }

            Client &sender = msg.getSender();
            const std::string pass = arg_vector[0];

            // If client is not in a channel, treat as server password (initial handshake).
            if (sender.getCurrentChannel().empty()) {
                if (sender.isPassCompleted()) {
                    std::cerr << ERROR << "PASS command already completed for client: " << sender.getNickname() << std::endl;
                    server.send(sender, ":server 462 * :You may not reregister");
                    return;
                }
                if (server.check_password(pass)) {
                    std::cout << SUCCESS << "Password accepted for client: " << sender.getFd() << std::endl;
                    sender.setPassCompleted(true);
                } else {
                    std::cerr << ERROR << "Incorrect server password for client: " << sender.getNickname() << std::endl;
                    server.send(sender, ":" + server.get_serverName() + " NOTICE * :Password incorrect");
                    sender.setPassCompleted(false);
                }
                return;
            }

            // Otherwise, treat as channel password (legacy behavior)
            Channel &channel = server.access_channel(sender.getCurrentChannel());
            if (channel.check_password(pass))
            {
                sender.setAuthenticated(true);
                std::cout << SUCCESS << "Channel password accepted for client: " << sender.getFd() << std::endl;
            }
            else
            {
                std::cerr << ERROR << "Incorrect channel password for client: " << sender.getNickname() << std::endl;
                close(sender.getSd());
                return;
            }
            break;
        }
        case CMD_NICK:
        {
            if (arg_vector.empty() || arg_vector[0].empty()) {
                std::cerr << ERROR << "NICK command requires a valid nickname" << std::endl;
                return;
            }
            Client &client = msg.getSender();
            const std::string newNickname = arg_vector[0];
            const std::string oldNickname = client.getNickname(); // snapshot old nick
            if (oldNickname == newNickname) {
                return;
            }
            // Check collisions
            for (size_t i = 0; i < server.get_clients().size(); ++i)
            {
                Client &c = server.get_clients()[i];
                if (c.getNickname() == newNickname) {
                    std::cerr << ERROR << "Nickname already taken: " << newNickname << std::endl;
                    server.send(client, ":server 433 * " + newNickname + " :Nickname is already in use");
                    return;
                }
            }
            const std::string prefix = ":" + (oldNickname.empty() ? "*" : oldNickname) + "!" + client.getUsername() + "@" + client.getIp();
            const std::string currentChannelName = client.getCurrentChannel();
            bool wasOp = false;
            Channel *chanPtr = NULL;
            if (!currentChannelName.empty()) {
                try {
                    chanPtr = &server.access_channel(currentChannelName);
                    wasOp = chanPtr->isOperator(client); // checks with OLD nickname
                } catch (...) {
                    chanPtr = NULL;
                }
            }
            client.setNickname(newNickname);
            std::cout << INFO << "Client nickname changed: " << oldNickname << " -> " << client.getNickname() << std::endl;
            if (chanPtr) {
                if (wasOp && !oldNickname.empty() && oldNickname != newNickname) {
                    chanPtr->removeOperator(oldNickname);
                    chanPtr->addOperator(client); // adds under NEW nickname
                }
                std::vector<int> &fds = chanPtr->getClients();
                for (size_t i = 0; i < fds.size(); ++i) {
                    Client *rcpt = find_client_by_fd(server, fds[i]);
                    if (rcpt) server.send(*rcpt, prefix + " NICK :" + newNickname);
                }
            } else {
                // Not in any channel yet: still tell the client so its UI updates
                server.send(client, prefix + " NICK :" + newNickname);
            }
            break;
        }
        case CMD_USER:
        {
            if (arg_vector.size() < 2 || arg_vector[0].empty() || arg_vector[1].empty()) {
                std::cerr << ERROR << "USER command requires a valid username and real name" << std::endl;
                return;
            }
            Client &client = msg.getSender();
            client.setUsername(arg_vector[0]);
            client.setRealName(arg_vector[1]);
            break;
        }
        case CMD_JOIN:
        {
            if (arg_vector.empty() || arg_vector[0].empty()) {
                std::cerr << ERROR << "JOIN command requires a valid channel name" << std::endl;
                return;
            }

            const std::string rawName = arg_vector[0];
            const std::string chan = strip_hash(rawName);
            const std::string keyArg = (arg_vector.size() > 1) ? arg_vector[1] : "";
            Client &client = msg.getSender();
            const std::string currentChannelName = client.getCurrentChannel();


            std::cout << "\n\n" << WARNING << "Current channel: " << currentChannelName << std::endl;
            std::cout << WARNING << "Joining channel: " << chan << std::endl;

            try {
                // If client is already in a channel, remove them from it (simple single-channel model)
                if (!currentChannelName.empty() && currentChannelName != chan) {
                    Channel &oldChannel = server.access_channel(currentChannelName);
                    oldChannel.removeClient(client);
                    std::cout << INFO << "Client " << client.getNickname() << " left channel " << currentChannelName << std::endl;
                }

                Channel &newChannel = server.access_channel(chan);

                // Checks: invite-only, key, limit
                if (newChannel.isInviteOnly() && !newChannel.isOperator(client) && !newChannel.isInvited(client.getNickname())) {
                    server.send(client, ":server 473 " + client.getNickname() + " " + ensure_hash(chan) + " :Cannot join channel (+i)");
                    return;
                }
                if (newChannel.hasKey() && !newChannel.check_password(keyArg)) {
                    server.send(client, ":server 475 " + client.getNickname() + " " + ensure_hash(chan) + " :Cannot join channel (+k)");
                    return;
                }
                if (newChannel.isFull()) {
                    server.send(client, ":server 471 " + client.getNickname() + " " + ensure_hash(chan) + " :Cannot join channel (+l)");
                    return;
                }

                // Already in?
                std::vector<int> &fdsRef = newChannel.getClients();
                for (size_t i = 0; i < fdsRef.size(); ++i) {
                    if (fdsRef[i] == client.getSd()) {
                        server.send(client, ":server 331 " + client.getNickname() + " " + ensure_hash(chan) + " :You are already in this channel");
                        return;
                    }
                }

                // Join
                newChannel.addClient(client);
                client.setCurrentChannel(chan);
                // consume invite if present
                if (newChannel.isInvited(client.getNickname()))
                    newChannel.revokeInvite(client.getNickname());

                // Broadcast JOIN
                const std::string vis = ensure_hash(chan);
                const std::string prefix = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getIp();
                std::vector<int> &clientFds = newChannel.getClients(); // AFTER addClient
                for (size_t i = 0; i < clientFds.size(); ++i) {
                    Client *rcpt = find_client_by_fd(server, clientFds[i]);
                    if (rcpt) server.send(*rcpt, prefix + " JOIN " + vis);
                }

                // Topic numerics
                server.send(client, ":server 332 " + client.getNickname() + " " + vis + " :" + newChannel.getTopic());
                server.send(client, ":server 333 " + client.getNickname() + " " + vis + " " + client.getNickname() + " " + get_current_timestamp());

                // Names
                std::string userList;
                for (size_t i = 0; i < clientFds.size(); ++i) {
                    Client *c = find_client_by_fd(server, clientFds[i]);
                    if (c) {
                        if (!userList.empty()) userList += " ";
                        std::string nick = c->getNickname();
                        if (newChannel.isOperator(*c))
                            userList += "@" + nick;
                        else
                            userList += nick;
                    }
                }
                server.send(client, ":server 353 " + client.getNickname() + " = " + vis + " :" + userList);
                server.send(client, ":server 366 " + client.getNickname() + " " + vis + " :End of /NAMES list");
            } catch (const std::runtime_error& e) {
                std::cerr << ERROR << "Error joining channel: " << e.what() << std::endl;
            }
            break;
        }
        case CMD_PRIVMSG:
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
                    std::vector<int> &fds = channel.getClients();
                    const std::string vis = ensure_hash(chan);
                    for (size_t i = 0; i < fds.size(); ++i) {
                        Client *rcpt = find_client_by_fd(server, fds[i]);
                        if (rcpt) {
                            server.send(*rcpt, prefix + " PRIVMSG " + vis + " :" + text);
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
            break;
        }
        case CMD_LIST:
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
            break;
        }
        case CMD_CAP:
        {
            if (arg_vector.empty() || arg_vector[0].empty())
            {
                std::cerr << ERROR << "CAP command requires a valid argument" << std::endl;
                return;
            }
            Client &client = msg.getSender();
            const std::string capCommand = arg_vector[0];

            if (capCommand == "LS")
            {
                server.send(client, ":" + server.get_serverName() + " CAP * LS :");
            }
            else if (capCommand == "REQ")
            {
                if (arg_vector.size() < 2 || arg_vector[1].empty())
                {
                    std::cerr << ERROR << "CAP REQ command requires a valid capability" << std::endl;
                    return;
                }
                // Keep simple ACK; add ':' if needed
                const std::string caps = (arg_vector[1][0] == ':') ? arg_vector[1] : ":" + arg_vector[1];
                server.send(client, ":" + server.get_serverName() + " CAP * ACK " + caps);
            }
            else if (capCommand == "END")
            {
                if (client.isAuthenticated())
                {
                    std::cerr << WARNING << "Client already authenticated: " << client.getNickname() << std::endl;
                    return;
                }
                if (!client.isPassCompleted())
                {
                    std::cerr << ERROR << "Client attempted CAP END without completing PASS" << std::endl;
                    server.send(client, ":" + server.get_serverName() + " NOTICE * :You must complete PASS before CAP END");
                    return;
                }
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
                server.send(client, ":" + server.get_serverName() + " 001 " + nick + " :Welcome to the IRC server, " + nick + "!");
                server.send(client, ":" + server.get_serverName() + " 002 " + nick + " :Your host is " + server.get_serverName() + ", running version 1.0");
                server.send(client, ":" + server.get_serverName() + " 004 " + nick + " " + server.get_serverName() + " 1.0 o o");
                server.send(client, ":" + server.get_serverName() + " 375 " + nick + " :- " + server.get_serverName() + " Message of the day -");
                server.send(client, ":" + server.get_serverName() + " 372 " + nick + " :- To what question is 42 an answer? ");
                server.send(client, ":" + server.get_serverName() + " 372 " + nick + " :- Type /join #general to start chatting.");
                server.send(client, ":" + server.get_serverName() + " 376 " + nick + " :End of /MOTD command");
            }
            else
            {
                std::cerr << ERROR << "Unknown CAP command: " << capCommand << std::endl;
                server.send(client, ":" + server.get_serverName() + " CAP * NAK :Unknown command");
            }
            break;
        }
        case CMD_KICK:
        {
            if (arg_vector.size() < 2) {
                std::cerr << ERROR << "KICK requires <channel> <nick> [reason]" << std::endl;
                return;
            }
            const std::string chan = strip_hash(arg_vector[0]);
            const std::string targetNick = arg_vector[1];
            std::string reason = "Kicked";
            size_t col = msg.getContent().find(" :");
            if (col != std::string::npos) reason = msg.getContent().substr(col + 2);

            try {
                Channel &channel = server.access_channel(chan);
                if (!channel.isOperator(msg.getSender())) {
                    server.send(msg.getSender(), ":server 482 " + msg.getSender().getNickname() + " " + ensure_hash(chan) + " :You're not channel operator");
                    return;
                }
                Client &target = get_client_from_channel_by_name(server, channel, targetNick); // CHANGED
                const std::string prefix = ":" + msg.getSender().getNickname() + "!" + msg.getSender().getUsername() + "@" + msg.getSender().getIp();
                const std::string vis = ensure_hash(chan);
                // Broadcast KICK
                std::vector<int> &fds = channel.getClients();
                for (size_t i = 0; i < fds.size(); ++i) {
                    Client *rcpt = find_client_by_fd(server, fds[i]);
                    if (rcpt)
                        server.send(*rcpt, prefix + " KICK " + vis + " " + targetNick + " :" + reason);
                }
                // Notify target and remove
                server.send(target, prefix + " KICK " + vis + " " + targetNick + " :" + reason);
                channel.removeClient(target);
                if (target.getCurrentChannel() == chan)
                    target.setCurrentChannel("");
            } catch (const std::exception &e) {
                std::cerr << ERROR << "KICK failed: " << e.what() << std::endl;
            }
            break;
        }
        case CMD_INVITE:
        {
            if (arg_vector.size() < 2) {
                std::cerr << ERROR << "INVITE requires <nick> <channel>" << std::endl;
                return;
            }
            const std::string targetNick = arg_vector[0];
            const std::string chan = strip_hash(arg_vector[1]);

            try {
                Channel &channel = server.access_channel(chan);
                if (!channel.isOperator(msg.getSender())) {
                    server.send(msg.getSender(), ":server 482 " + msg.getSender().getNickname() + " " + ensure_hash(chan) + " :You're not channel operator");
                    return;
                }
                Client *target = find_client_by_nick(server, targetNick);
                if (!target) {
                    server.send(msg.getSender(), ":server 401 " + msg.getSender().getNickname() + " " + targetNick + " :No such nick");
                    return;
                }
                // Do not invite if already in channel
                std::vector<int> &fds = channel.getClients();
                for (size_t i = 0; i < fds.size(); ++i) {
                    Client *c = find_client_by_fd(server, fds[i]);
                    if (c && c->getNickname() == targetNick) {
                        server.send(msg.getSender(), ":server 443 " + msg.getSender().getNickname() + " " + targetNick + " " + ensure_hash(chan) + " :is already on channel");
                        return;
                    }
                }
                channel.invite(targetNick);
                server.send(msg.getSender(), ":server 341 " + msg.getSender().getNickname() + " " + targetNick + " " + ensure_hash(chan));
                const std::string prefix = ":" + msg.getSender().getNickname() + "!" + msg.getSender().getUsername() + "@" + msg.getSender().getIp();
                server.send(*target, prefix + " INVITE " + targetNick + " :" + ensure_hash(chan));
            } catch (const std::exception &e) {
                std::cerr << ERROR << "INVITE failed: " << e.what() << std::endl;
            }
            break;
        }
        case CMD_TOPIC:
        {
            if (arg_vector.empty()) {
                std::cerr << ERROR << "TOPIC requires <channel> [ :topic ]" << std::endl;
                return;
            }
            const std::string chan = strip_hash(arg_vector[0]);
            size_t col = msg.getContent().find(" :");
            bool setTopic = (col != std::string::npos);
            std::string newTopic = setTopic ? msg.getContent().substr(col + 2) : "";

            try {
                Channel &channel = server.access_channel(chan);
                const std::string vis = ensure_hash(chan);
                if (!setTopic) {
                    // View
                    server.send(msg.getSender(), ":server 332 " + msg.getSender().getNickname() + " " + vis + " :" + channel.getTopic());
                    server.send(msg.getSender(), ":server 333 " + msg.getSender().getNickname() + " " + vis + " " + msg.getSender().getNickname() + " " + get_current_timestamp());
                    return;
                }
                // Set
                if (channel.isTopicOpOnly() && !channel.isOperator(msg.getSender())) {
                    server.send(msg.getSender(), ":server 482 " + msg.getSender().getNickname() + " " + vis + " :You're not channel operator");
                    return;
                }
                channel.setTopic(newTopic);
                const std::string prefix = ":" + msg.getSender().getNickname() + "!" + msg.getSender().getUsername() + "@" + msg.getSender().getIp();
                std::vector<int> &fds = channel.getClients();
                for (size_t i = 0; i < fds.size(); ++i) {
                    Client *rcpt = find_client_by_fd(server, fds[i]);
                    if (rcpt)
                        server.send(*rcpt, prefix + " TOPIC " + vis + " :" + newTopic);
                }
            } catch (const std::exception &e) {
                std::cerr << ERROR << "TOPIC failed: " << e.what() << std::endl;
            }
            break;
        }
        case CMD_MODE:
        {
            if (arg_vector.empty()) {
                std::cerr << ERROR << "MODE requires <channel> [modes] [params]" << std::endl;
                return;
            }
            const std::string chan = strip_hash(arg_vector[0]);
            try {
                Channel &channel = server.access_channel(chan);
                if (arg_vector.size() == 1) {
                    // Could list current modes (minimal echo)
                    server.send(msg.getSender(), ":server 324 " + msg.getSender().getNickname() + " " + ensure_hash(chan) + " :");
                    server.send(msg.getSender(), ":server 332 " + msg.getSender().getNickname() + " " + ensure_hash(chan) + " :" + channel.getTopic());
                    return;
                }
                if (!channel.isOperator(msg.getSender())) {
                    server.send(msg.getSender(), ":server 482 " + msg.getSender().getNickname() + " " + ensure_hash(chan) + " :You're not channel operator");
                    return;
                }
                // Parse mode string like +it-k key -o nick +l 10
                std::string modeStr = arg_vector[1];
                int sign = 0; // +1 for '+', -1 for '-'
                size_t paramIndex = 2;
                std::string applied; // to echo/broadcast

                const std::string prefix = ":" + msg.getSender().getNickname() + "!" + msg.getSender().getUsername() + "@" + msg.getSender().getIp();
                const std::string vis = ensure_hash(chan);

                for (size_t i = 0; i < modeStr.size(); ++i) {
                    char c = modeStr[i];
                    if (c == '+') { sign = 1; applied += "+"; continue; }
                    if (c == '-') { sign = -1; applied += "-"; continue; }
                    if (sign == 0) continue;
                    switch (c) {
                    case 'i':
                        channel.setInviteOnly(sign > 0);
                        applied += "i";
                        break;
                    case 't':
                        channel.setTopicOpOnly(sign > 0);
                        applied += "t";
                        break;
                    case 'k':
                        if (sign > 0) {
                            if (paramIndex >= arg_vector.size()) break;
                            channel.setKey(arg_vector[paramIndex++]);
                            applied += "k";
                        } else {
                            channel.clearKey();
                            applied += "k";
                        }
                        break;
                    case 'o': { // FIX: add scope to contain variable declarations
                        if (paramIndex >= arg_vector.size()) break;
                        std::string nick = arg_vector[paramIndex++];
                        Client *target = find_client_by_nick(server, nick);
                        if (target) {
                            if (sign > 0) channel.addOperator(*target);
                            else channel.removeOperator(nick);
                            applied += "o";
                        }
                        break;
                    }
                    case 'l':
                        if (sign > 0) {
                            if (paramIndex >= arg_vector.size()) break;
                            int lim = atoi(arg_vector[paramIndex++].c_str());
                            channel.setLimit(lim);
                            applied += "l";
                        } else {
                            channel.setLimit(-1);
                            applied += "l";
                        }
                        break;
                    }
                }
                // Broadcast MODE change to channel
                if (!applied.empty()) {
                    std::string line = prefix + " MODE " + vis + " " + applied;
                    std::vector<int> &fds = channel.getClients();
                    for (size_t i = 0; i < fds.size(); ++i) {
                        Client *rcpt = find_client_by_fd(server, fds[i]);
                        if (rcpt)
                            server.send(*rcpt, line);
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << ERROR << "MODE failed: " << e.what() << std::endl;
            }
            break;
        }
        case CMD_PING:
        {
            std::cout << INFO << "Received PING from " << msg.getSender().getNickname() << std::endl;
            const std::string token = (arg_vector.empty() ? server.get_serverName() : arg_vector[0]);
            std::string response = "PONG :" + token + "\r\n";
            server.send(msg.getSender(), response);
            break;
        }
        case CMD_WHO:
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
                std::vector<int> &fds = channel.getClients();
                for (size_t i = 0; i < fds.size(); ++i) {
                    Client *c = find_client_by_fd(server, fds[i]);
                    if (c)
                    {
                        std::string response = ":server 352 " + client.getNickname() + " " + ensure_hash(chan) + " " +
                                               c->getUsername() + " " +
                                               c->getIp() + " server " +
                                               c->getNickname() + " H :0 " +
                                               c->getRealName() + "\r\n";
                        server.send(client, response);
                    }
                }
                server.send(client, ":server 315 " + client.getNickname() + " " + ensure_hash(chan) + " :End of /WHO list\r\n");
            } catch (const std::runtime_error& e) {
                std::cerr << ERROR << "Error processing WHO command: " << e.what() << std::endl;
                server.send(client, ":server 401 " + client.getNickname() + " " + ensure_hash(chan) + " :No such channel\r\n");
            }
            break;
        }
        case CMD_QUIT:
        {
            Client &client = msg.getSender();
            std::cout << WARNING << "Client " << client.getNickname() << " is disconnecting." << std::endl;
            close(client.getSd());
            server.remove_client(client.getSd());
            return;
        }
        default:
        {
            std::cerr << ERROR << "Unknown command: " << command << std::endl;
            server.send(msg.getSender(), "ERROR :Unknown command: " + command);
            break;
        }
    }
}