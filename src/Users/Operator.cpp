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
    
}

void Operator::invite(Channel& channel, const Connection& client)
{

}
void Operator::topic(Channel& channel, const std::string& topic)
{

}
void Operator::mode(Channel& channel)
{

}