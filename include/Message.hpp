/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 12:12:56 by root              #+#    #+#             */
/*   Updated: 2025/04/09 19:41:47 by root             ###   ########.fr       */
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

class Message
{
    private:
        std::string _sender;
        std::string _content;
        std::time_t _timestamp;
        int         _bytesRead;

    public:
        Message();
        Message(std::string sender, std::string content, std::time_t timestamp = std::time(0), int bytesRead = 0);
        Message(const Message &msg);
        ~Message();
        Message &operator=(const Message &msg);

        bool    isValid() const;
        bool    isEmpty() const;

        std::string getSender() const;
        std::string getContent() const;
        std::time_t getTimestamp() const;
        int         getBytesRead() const;

        void setSender(std::string sender);
        void setContent(std::string content);
        
        friend std::ostream &operator<<(std::ostream &os, const Message &msg);
};

#endif