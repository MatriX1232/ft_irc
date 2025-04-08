/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 12:28:49 by root              #+#    #+#             */
/*   Updated: 2025/04/08 14:40:37 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/utils.hpp"
#include "../include/Message.hpp"
#include "../include/Channel.hpp"
#include <string>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        ft_log("Usage: ./ircserv [port] [password]", "", 3);
        return (-1);
    }

    Server server = Server(atoi(argv[1]));
    server.listen(1);
    Message msg = server.recv();
    while (msg.isValid())
    {
        if (msg.isEmpty())
        {
            std::cout << "Client disconnected" << std::endl;
            break;
        }
        std::cout << msg << std::endl;
        msg = server.recv();
    }
    std::cout << "Exiting server..." << std::endl;
    server.disconnect();

    return 0;
}