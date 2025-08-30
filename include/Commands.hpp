#ifndef COMMANDS_HPP
# define COMMANDS_HPP

#include <iostream>
#include <string>
#include <vector>
#include "Message.hpp"
#include "Server.hpp"
#include "Client.hpp"

void    parse_message(Server &server, Message &msg);
void    parse_initial_message(Server &server, Client &client, std::string cmd);

#endif