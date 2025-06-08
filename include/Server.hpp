/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 23:47:53 by root              #+#    #+#             */
/*   Updated: 2025/06/08 14:59:22 by root             ###   ########.fr       */
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
#include <poll.h>
#include <vector>
#include "Headers.hpp"
#include "Message.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Server
{
    private:
        int					    _port;
        int					    _serverSd;
        int					    _newSd;
        std::string			    _password;
        std::string			    _serverName;
        std::vector<Client>	    _clients;
        std::vector<Channel>    _channels;

        void    retrieve_initial_info(Client &client);
    
    public:
        Server(int port);
        ~Server();
        
        int start_listening(int n_clients);
        int accept_new_client();
        // int listen(int n_clients);
        int disconnect();
        void send(Client &client, std::string msg);
        Message recv(Client &client);

        int recv_file();

        int get_port();
        int get_serverSd();
        int getListenFd() const;
        int getFd() const;
		std::vector<Client> get_clients() const;
        std::vector<Channel> &get_channels();

        void    add_channel(Channel &channel);
        Channel &access_channel(std::string channelName);
        
        bool    check_password(std::string password);
        int     assign_read_mode(int listen_fd, fd_set &readfds);

        friend std::ostream &operator<<(std::ostream &os, const Server &server);
};


#endif