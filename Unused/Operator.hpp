#ifndef OPERATOR_HPP
#define OPERATOR_HPP

#include "../Channels/Channel.hpp"

class Operator
{
public:
    Operator();
    ~Operator();

    void        kick(Channel& channel, const Connection& client);
    void        invite(Channel& channel, const Connection& client);
    void        topic(Channel& channel, const std::string& topic);
    void        mode(Channel& channel);
    
};

#endif