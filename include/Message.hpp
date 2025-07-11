/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 12:12:56 by root              #+#    #+#             */
/*   Updated: 2025/06/29 15:45:38 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
# define MESSAGE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../include/Headers.hpp"
#include "../include/Client.hpp"

class Message
{
    private:
        Client    _sender;
        std::string     _nickname;
        std::string     _content;
        std::time_t     _timestamp;
        int             _bytesRead;

    public:
        Message();
        Message(Client sender, std::string nickname, const std::string& content, std::time_t timestamp = std::time(0), int bytesRead = 0);
        Message(const Message &msg);
        ~Message();

        Message &operator=(const Message &msg);

        bool    isValid() const;
        bool    isEmpty() const;

        Client      &getSender();
        std::string getNickname() const;
        std::string getContent() const;
        std::time_t getTimestamp() const;
        int         getBytesRead() const;

        void setSender(Client &sender);
        void setContent(std::string content);

        friend std::ostream &operator<<(std::ostream &os, const Message &msg);
};

#endif
