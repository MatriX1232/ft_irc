#ifndef COMMANDS_HPP
# define COMMANDS_HPP

#include <iostream>
#include <string>
#include <vector>
#include "Message.hpp"
#include "Server.hpp"

void    parse_message(Server &server, Message &msg);

#endif