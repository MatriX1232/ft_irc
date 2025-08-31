#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <map>
#include <vector>
#include <ctime>
#include <set>
#include "Message.hpp"
#include "Client.hpp"

class Channel
{
    private:
        std::vector<Message> _messages;
        std::vector<Client*> _clients;
        std::string _name;
        std::string _topic;
        std::string _password;   // channel key (MODE +k)

        // Channel modes/state
        bool                _inviteOnly;     // MODE +i
        bool                _topicOpOnly;    // MODE +t
        int                 _userLimit;      // MODE +l (<=0 => no limit)
        std::set<std::string> _operators;    // nicks with chanop (+o)
        std::set<std::string> _invited;      // invited nicks (for +i)

    public:
        Channel();
        Channel(std::string name = "", std::string topic = "", std::string password = "");
        ~Channel();

        bool check_password(const std::string password) const;

        void addMessage(Message msg);
        void addClient(Client &client);
        void removeClient(Client &client);
        void displayMessages();
        void clearMessages();
        
        std::string getName() const;
        std::string getTopic() const;
        void        setTopic(const std::string &topic);
        std::vector<Client*>& getClients();
        const std::vector<Client*>& get_clients() const;

        std::vector<Message> getMessages();
        Message getMessageByIndex(int index);
        std::vector<Message> getMessageByUser(std::string userName);
        std::vector<Message> getMessageByContent(std::string messageContentFragment);

        // Mode helpers
        bool    isInviteOnly() const;
        void    setInviteOnly(bool on);
        bool    isTopicOpOnly() const;
        void    setTopicOpOnly(bool on);
        bool    hasKey() const;
        void    setKey(const std::string &key);
        void    clearKey();
        void    setLimit(int limit);     // <=0 clears the limit
        int     getLimit() const;
        bool    isFull() const;

        // Operators
        bool    isOperator(const Client &client) const;
        void    addOperator(Client &client);
        void    removeOperator(const std::string &nick);

        // Invites
        void    invite(const std::string &nick);
        bool    isInvited(const std::string &nick) const;
        void    revokeInvite(const std::string &nick);

        friend std::ostream &operator<<(std::ostream &os, const Channel &channel);
};

#endif