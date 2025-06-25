#include "Operator.hpp"

Operator::Operator()
{
     #ifdef DEBUG
        std::cout << "Operator default constructor called\n";
    #endif
}

Operator::~Operator()
{
    #ifdef DEBUG
        std::cout << "User destructor called\n";
    #endif
}

void Operator::kick(Channel& channel, const Connection& client)
{
    (void)channel;
    (void)client;
    std::cout << "todo\n";
}

void Operator::invite(Channel& channel, const Connection& client)
{
    (void)channel;
    (void)client;
    std::cout << "todo\n";
}

void Operator::topic(Channel& channel, const std::string& topic)
{
    (void)channel;
    (void)topic;
    std::cout << "todo\n";
}

void Operator::mode(Channel& channel)
{
    (void)channel;
    std::cout << "todo\n";
}