/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 00:29:45 by root              #+#    #+#             */
/*   Updated: 2025/04/08 13:50:55 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <map>
#include <vector>
#include <ctime>

class Channel
{
    private:
        // vector < map <time, userName, messageContent> > _messages;
        std::vector<std::map<time_t, std::string, std::string> > _messages;
    
    public:
        Channel();
        ~Channel();
        void addMessage(std::string userName, std::string messageContent);
        void displayMessages();
        void clearMessages();
        std::vector<std::map<std::string, std::string> > getMessages();
        std::map<time_t, std::string, std::string> getMessageByIndex(int index);
        std::map<time_t, std::string, std::string> getMessageByUser(std::string userName);
        std::map<time_t, std::string, std::string> getMessageByTime(time_t time);
        std::map<time_t, std::string, std::string> getMessageByContent(std::string messageContentFragment);
};

#endif