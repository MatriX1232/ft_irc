#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <string>
#include "../include/utils.hpp"
#include "../include/Headers.hpp"
#include "../include/Channel.hpp"

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos)
    {
        token = str.substr(start, end - start);
        tokens.push_back(token);
        start = end + 1;
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));

    return tokens;
}

std::string c_strip(char *str)
{
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t'))
        end--;
    *(end + 1) = '\0';
    return str;
}

std::string append_number(const std::string &str, int number)
{
    std::stringstream ss;
    ss << str << number;
    return ss.str();
}

std::string get_current_timestamp()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

Client &get_client_from_channel_by_name(Channel &channel, const std::string &client_name)
{
    std::vector<Client> clients = channel.getClients();
    for (int i = 0; i < (int)clients.size(); ++i)
    {
        if (clients[i].getNickname() == client_name)
        {
            return clients[i];
        }
    }
    throw std::runtime_error("Client not found");
}

// Client &get_client_from_msg(const Message &msg)
// {
//     Client &client = msg.getSender();
//     if (client.isEmpty())
//     {
//         std::cerr << ERROR << "Client is empty in message" << std::endl;
//         throw std::runtime_error("Client is empty in message");
//     }
//     return const_cast<Client &>(client);
// }