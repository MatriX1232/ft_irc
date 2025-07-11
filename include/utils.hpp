/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 11:56:28 by root              #+#    #+#             */
/*   Updated: 2025/06/29 15:47:29 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <sys/select.h>
#include "Client.hpp"
#include "Message.hpp"

void ft_log(const std::string &str, const std::string &str_optional, int level);
void ft_log_sub(const std::string &str, const std::string &str_optional, int level, int sub_level);
std::vector<std::string> split(const std::string &str, char delimiter);
std::string c_strip(char *str);


// socket utils
int    wait_for_activity(int maxfd, fd_set &readfds);

// string utils
std::string append_number(const std::string &str, int number);
std::string get_current_timestamp();

// client utils
// Client &get_client_from_msg(const Message &msg);

#endif