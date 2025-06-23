#include "Channel.hpp"

Channel::Channel() :    name(""),
                        topic(""),
                        mode(0)
{
    #ifdef DEBUG
        std::cout << "Channel default constructor called\n";
    #endif
}

Channel::~Channel()
{
    #ifdef DEBUG
        std::cout << "Channel destructor called\n";
    #endif
}

const std::string&  Channel::getName() const
{
    return (name);
}

const std::string&  Channel::getTopic() const
{
    return (topic);
}

int                 Channel::getMode() const
{
    return (mode);
}

std::map<std::string, Connection&>  Channel::getMembers() const
{
    return (members);
}

void    Channel::setName(const std::string& name)
{
    this->name = name;
}

void    Channel::setTopic(const std::string& topic)
{
    this->topic = topic;
}

void    Channel::setMode(int flags)
{
    mode |= flags;
}

