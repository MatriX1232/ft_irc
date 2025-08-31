#include "../include/Channel.hpp"
#include "../include/Headers.hpp"
#include <algorithm> // Required for std::remove_if

Channel::Channel() : _name(""), _topic(""), _password(""), _inviteOnly(false), _topicOpOnly(false), _userLimit(-1)
{
    
}

Channel::Channel(std::string name, std::string topic, std::string password)
    : _name(name), _topic(topic), _password(password), _inviteOnly(false), _topicOpOnly(false), _userLimit(-1)
{
    // Constructor implementation
}

Channel::~Channel()
{
    // Destructor implementation
}

void Channel::addClient(Client &client)
{
    // avoid duplicates
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i] && _clients[i]->getSd() == client.getSd())
            return;
    }
    _clients.push_back(&client);
    // Auto-op first user joining an empty channel
    if (_clients.size() == 1)
        addOperator(client);
}

void Channel::removeClient(Client &client)
{
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i] && _clients[i]->getSd() == client.getSd())
        {
            _clients.erase(_clients.begin() + i);
            // Do not forcibly drop operator status here; keep as is or manage elsewhere.
            std::cout << INFO << "Client removed from channel: " << _name << std::endl;
            return;
        }
    }
    std::cerr << WARNING << "Client not found in channel: " << _name << std::endl;
}

std::vector<Client*>& Channel::getClients()
{
    return _clients;
}

const std::vector<Client*>& Channel::get_clients() const
{
    return this->_clients;
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

std::string Channel::getName() const
{
    return _name;
}

std::string Channel::getTopic() const
{
    return _topic;
}

void Channel::setTopic(const std::string &topic)
{
    _topic = topic;
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
        if (msg.getSender().getNickname() == userName)
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

bool Channel::check_password(const std::string password) const
{
    return _password == password;
}

// Mode helpers
bool Channel::isInviteOnly() const { return _inviteOnly; }
void Channel::setInviteOnly(bool on) { _inviteOnly = on; }

bool Channel::isTopicOpOnly() const { return _topicOpOnly; }
void Channel::setTopicOpOnly(bool on) { _topicOpOnly = on; }

bool Channel::hasKey() const { return !_password.empty(); }
void Channel::setKey(const std::string &key) { _password = key; }
void Channel::clearKey() { _password.clear(); }

void Channel::setLimit(int limit) { _userLimit = limit <= 0 ? -1 : limit; }
int  Channel::getLimit() const { return _userLimit; }
bool Channel::isFull() const
{
    if (_userLimit <= 0) return false;
    return (int)_clients.size() >= _userLimit;
}

// Operators
bool Channel::isOperator(const Client &client) const
{
    return _operators.find(client.getNickname()) != _operators.end();
}

void Channel::addOperator(Client &client)
{
    if (!client.getNickname().empty())
        _operators.insert(client.getNickname());
}

void Channel::removeOperator(const std::string &nick)
{
    std::set<std::string>::iterator it = _operators.find(nick);
    if (it != _operators.end())
        _operators.erase(it);
}

// Invites
void Channel::invite(const std::string &nick)
{
    if (!nick.empty())
        _invited.insert(nick);
}

bool Channel::isInvited(const std::string &nick) const
{
    return _invited.find(nick) != _invited.end();
}

void Channel::revokeInvite(const std::string &nick)
{
    std::set<std::string>::iterator it = _invited.find(nick);
    if (it != _invited.end())
        _invited.erase(it);
}

std::ostream &operator<<(std::ostream &os, const Channel &channel)
{
    os << CYAN << "--------------------------" << END << std::endl;
    os << CYAN << "Channel Name: " << END << channel._name << std::endl;
    os << CYAN << "Topic: " << END << channel._topic << std::endl;
    os << RED << "Password: " << channel._password << std::endl;
    os << CYAN << "Messages: " << END << std::endl;
    for (int i = 0; i < (int)channel._messages.size(); i++)
        os << channel._messages[i] << std::endl;
    os << CYAN << "--------------------------" << END << std::endl;
    return os;
}