#include "GandhiBender.hpp"

GandhiBender::GandhiBender(const std::string& pass, const char* port) : Bender(pass, "GandhiBender", port)
{
    #ifdef DEBUG
        std::cout << "GandhiBender constructor called\n";
    #endif
}

GandhiBender::~GandhiBender()
{
    if(connectionFd != -1)
        close(connectionFd);
}

void GandhiBender::handleModeChange(const std::vector<std::string>& msg)
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
        enqueueMsg("PRIVMSG " + args.channel + " :In a gentle way, you can shake the world\r\n");
}

void GandhiBender::handlePrivateMsg(const std::vector<std::string>& msg)
{
    std::cout << "todo\n";
}

void GandhiBender::handleChannelMsg(const std::vector<std::string>& msg)
{
    std::cout << "todo\n";
}
