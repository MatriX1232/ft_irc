#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"
#include "Server.hpp"


void ft_log(const std::string &str, const std::string &str_optional, int level);
void ft_log_sub(const std::string &str, const std::string &str_optional, int level, int sub_level);
std::vector<std::string> split(const std::string &str, char delimiter);
std::string c_strip(char *str);


// string utils
std::string append_number(const std::string &str, int number);
std::string get_current_timestamp();

Client &get_client_from_channel_by_name(Server &server, Channel &channel, const std::string &client_name);


#endif