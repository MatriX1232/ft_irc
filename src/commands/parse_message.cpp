#include <iostream>
#include <string>
#include <vector>
#include "../../include/Message.hpp"
#include "../../include/Commands.hpp"
#include "../../include/Server.hpp"
#include "../../include/utils.hpp"


void    parse_message(Message &msg)
{
    std::string content = msg.getContent();
    std::string command = content.substr(0, content.find(" "));
    std::string args = content.substr(content.find(" ") + 1);
    std::vector<std::string> arg_vector = split(args, ' ');

    if (command == "PASS")
    {
        ft_log("PASS", "Password: " + arg_vector[0], 0);
    }
    else if (command == "NICK")
    {
        ft_log("NICK", "Nickname: " + arg_vector[0], 0);
    }
    else if (command == "USER")
    {
        ft_log("USER", "Username: " + arg_vector[0], 0);
    }
    else
    {
        ft_log("Unknown command", "Command: " + command, 2);
    }
}
