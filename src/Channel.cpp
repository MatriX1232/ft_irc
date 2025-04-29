/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/09 19:43:32 by root              #+#    #+#             */
/*   Updated: 2025/04/29 16:59:45 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"

Channel::Channel(std::string name, std::string topic, std::string password) : _name(name), _topic(topic), _password(password)
{
    // Constructor implementation
}

Channel::~Channel()
{
    // Destructor implementation
}
void Channel::addMessage(Message msg)
{
    _messages.push_back(msg);
}

void Channel::displayMessages()
{
    for (int i=0; i < (int)_messages.size(); i++)
    {
        std::cout << _messages[i] << std::endl;
    }
}

void Channel::clearMessages()
{
    _messages.clear();
}

std::vector<Message> Channel::getMessages()
{
    return _messages;
}

Message Channel::getMessageByIndex(int index)
{
    if (index < 0 || index >= (int)_messages.size())
        return Message();
    return _messages[index];
}

std::vector<Message> Channel::getMessageByUser(std::string userName)
{
    std::vector<Message> result;
    for (int i = 0; i < (int)_messages.size(); i++)
    {
        Message msg = _messages[i];
        if (msg.getSender() == userName)
            result.push_back(msg);
    }
    return result;
}

std::vector<Message> Channel::getMessageByContent(std::string messageContentFragment)
{
    std::vector<Message> result;
    for (int i = 0; i < (int)_messages.size(); i++)
    {
        Message msg = _messages[i];
        // Check if the message content contains the specified fragment
        // Use find() to check for substring presence
        if (msg.getContent().find(messageContentFragment) != std::string::npos)
            result.push_back(msg);
    }
    return result;
}

std::ostream &operator<<(std::ostream &os, const Channel &channel)
{
    os << CYAN << "Channel Name: " << END << channel._name << std::endl;
    os << CYAN << "Topic: " << END << channel._topic << std::endl;
    os << RED << "Password: " << channel._password << std::endl;
    os << CYAN << "Messages: " << END << std::endl;
    for (int i = 0; i < (int)channel._messages.size(); i++)
        os << channel._messages[i] << std::endl;
    return os;
}