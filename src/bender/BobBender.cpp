#include "BobBender.hpp"

BobBender::BobBender(const std::string& pass, const char* port) : Bender(pass, "BobBender", port)
{
    #ifdef DEBUG
        std::cout << "BobBender constructor called\n";
    #endif
}

BobBender::~BobBender()
{
    if(connectionFd != -1)
        close(connectionFd);
}

void BobBender::handleModeChange(const std::vector<std::string>& msg)
{
    modeArgs args;

    args.channel = msg.at(2);
    args.modeChanged = msg.at(3);
    args.target = msg.at(4);
    
    if (args.modeChanged == ":+o")
    {
        if(args.target == name)
            channels.insert(args.channel);
    }
    else if (args.modeChanged == ":-o")
    {
        if (args.target == name)
            channels.erase(args.channel);
    }
    else
        enqueueMsg("PRIVMSG " + args.channel + " :Mhhh, ok!\r\n");
}

void BobBender::handlePrivateMsg(const std::vector<std::string>& msg)
{
    std::cout << "todo\n";
}

void BobBender::handleChannelMsg(const std::vector<std::string>& msg)
{
    const std::string& sender = msg.at(0).substr(1, msg.at(0).find('!'));
    const std::string& channel = msg.at(2);
    const std::string& message = msg.at(3);

    std::cout << "message : " << message << '\n';
    if (message.compare(0, 2, ":!") == 0)
    {
        if (message.compare(0, 7, ":!kick") == 0)
            enqueueMsg("PRIVMSG " + channel + " :Hi! " + sender + " you should ask SatanBender to do that!\r\n");
        else if (message.compare(0, 11, ":!datetime") == 0)
            enqueueMsg("PRIVMSG " + channel + " " + getTimeStamp() + "\r\n");
    }
    else
        enqueueMsg("PRIVIMSG " + channel + " :ciao!\r\n");
}

