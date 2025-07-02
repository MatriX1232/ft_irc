/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 12:29:50 by root              #+#    #+#             */
/*   Updated: 2025/06/29 15:51:01 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Message.hpp"
#include "../include/Client.hpp"
#include "../include/utils.hpp"

Message::Message() 
    : _sender(Client()), _nickname(""), _content(""), _timestamp(std::time(0)), _bytesRead(0)
{
    // Default constructor implementation
}
Message::Message(const Message &msg) 
    : _sender(msg._sender), _nickname(msg._nickname), _content(msg._content), _timestamp(msg._timestamp), _bytesRead(msg._bytesRead)
{
    // Copy constructor implementation
}

Message::Message(Client sender, std::string nickname, const std::string& content, std::time_t timestamp, int bytesRead)
    : _sender(sender), _nickname(nickname), _content(content), _timestamp(timestamp), _bytesRead(bytesRead)
{
    // Parameterized constructor implementation
}

Message::~Message()
{
    // Destructor implementation
}

Client& Message::getSender()
{
    return _sender;
}

std::string Message::getNickname() const
{
    return _nickname;
}

std::string Message::getContent() const
{
    return _content;
}

std::time_t Message::getTimestamp() const
{
    return _timestamp;
}

int Message::getBytesRead() const
{
    return _bytesRead;
}

void Message::setSender(Client &sender)
{
    _sender = sender;
}

void Message::setContent(std::string content)
{
    _content = content;
}

std::ostream& operator<<(std::ostream &os, const Message &msg)
{
    std::time_t timestamp = msg.getTimestamp();
    std::string strTime = strtok(std::ctime(&timestamp), "\n");
    os << CYAN;
    os << "<" << strTime;
    os << BOLD;
    os << " | " << msg.getNickname() << " | ";
    os << END << CYAN;
    os << msg.getBytesRead() << " [bytes]" << ">  ";
    os << END;
    os << msg.getContent();

    return os;
}

bool Message::isValid() const
{
    return !_content.empty();
}

bool Message::isEmpty() const
{
    return _content.empty();
}

Message& Message::operator=(const Message &msg)
{
    if (this != &msg)
    {
        _sender = msg._sender;
        _content = msg._content;
        _timestamp = msg._timestamp;
        _bytesRead = msg._bytesRead;
    }
    return *this;
}