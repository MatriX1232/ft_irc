/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 23:47:53 by root              #+#    #+#             */
/*   Updated: 2025/04/08 12:50:47 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #pragma ones

#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include "../include/Headers.hpp"
#include "Message.hpp"

class Server
{
    private:
        int         _port;
        int         _serverSd;
        int         _newSd;
        std::string _password;
        std::string _serverName;
    
    public:
        Server(int port);
        ~Server();
        
        int listen(int n_clients);
        int disconnect();
        int send(std::string msg);
        Message recv();

        int recv_file();

        int get_port();
        int get_serverSd();
        
        bool    check_password(std::string password);
};


#endif