#include "SatanBender.hpp"

SatanBender::SatanBender(const std::string& pass, const char* port) : Bender(pass, "SatanBender", port)
{
    #ifdef DEBUG
        std::cout << "SatanBender constructor called\n";
    #endif
}

SatanBender::~SatanBender()
{
    if(connectionFd != -1)
        close(connectionFd);
}

void SatanBender::handleModeChange(const std::vector<std::string>& msg)
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
        enqueueMsg("PRIVMSG " + args.channel + " :STOP CHANGING MODES YOU STUP** MOT****[redacted]\r\n");
}

void SatanBender::handlePrivateMsg(const std::vector<std::string>& msg)
{
    std::cout << "todo\n";
}

void SatanBender::handleChannelMsg(const std::vector<std::string>& msg)
{
    std::cout << "todo\n";
}