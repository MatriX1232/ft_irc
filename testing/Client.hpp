/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 23:47:53 by root              #+#    #+#             */
/*   Updated: 2025/03/09 11:38:11 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #pragma ones

#ifndef CLIENT_HPP
# define CLIENT_HPP

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

class Client
{
    private:
        int             _port;
        std::string     _ip;
        int             _clientSd;
        char            _msg[150];
    
    public:
        Client(std::string ip, int port);
        ~Client();
        
        int try_connect();
        int disconnect();
        int send(std::string msg);
        int recv();

};


#endif