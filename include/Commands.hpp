/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/09 19:28:11 by root              #+#    #+#             */
/*   Updated: 2025/05/03 00:21:22 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
# define COMMANDS_HPP

#include <iostream>
#include <string>
#include <vector>
#include "Message.hpp"
#include "Server.hpp"

void parse_message(Server &server, Message &msg);

#endif