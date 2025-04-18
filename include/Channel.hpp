/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 00:29:45 by root              #+#    #+#             */
/*   Updated: 2025/04/16 00:02:12 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <map>
#include <vector>
#include <ctime>
#include "Message.hpp"
#include "Client.hpp"

class Channel
{
    private:
        std::vector<Message> _messages;
        std::vector<Client> _clients;
        std::string _name;
        std::string _topic;
        std::string _password;
    
    public:
        Channel(std::string name = "", std::string topic = "", std::string password = "");
        ~Channel();
        void addMessage(Message msg);
        void displayMessages();
        void clearMessages();
        std::vector<Message> getMessages();
        Message getMessageByIndex(int index);
        std::vector<Message> getMessageByUser(std::string userName);
        std::vector<Message> getMessageByContent(std::string messageContentFragment);
        friend std::ostream &operator<<(std::ostream &os, const Channel &channel);
};

#endif