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
    
    if (args.modeChanged == ":+o" && args.target == name)
    {
        channelsOperated.insert(args.channel);
    }
    else if (args.modeChanged == ":-o" && args.target == name)
    {
        channelsOperated.erase(args.channel);
    }
    else
        enqueueMsg("PRIVMSG " + args.channel + " :Mhhh, ok!\r\n");
}

void BobBender::handlePrivateMsg(const std::vector<std::string>& msg)
{
    std::string sender(msg.at(0).substr(1, msg.at(0).find('!') - 1));

    enqueueMsg("PRIVMSG " + sender + " :Ciao!\r\n");
}

void BobBender::handleLastSeen(const std::string& chanName,
                            const ChannelInfo* chan,
                            const std::vector<std::string>& msg)
{
    time_t  lastInteraction;
    
    try
    {
        lastInteraction =  chan->getLastSeen(msg.at(4));
        if (!lastInteraction)
            enqueueMsg("PRIVMSG " + chanName + " :" + msg.at(4) + " Never seen on this channel\r\n");
        else
            enqueueMsg("PRIVMSG " + chanName + " :Last time seen -> " 
                        + epochToTimeStamp(lastInteraction) + "\r\n");
    }
    catch(const std::exception& e)
    {
        enqueueMsg("PRIVMSG " + chanName + " :Ah Ah.. Give me a user to check!\r\n");
    }
}

void BobBender::handleChannelMsg(const std::vector<std::string>& msg)
{
    const std::string& sender = msg.at(0).substr(1);
    const std::string& channel = msg.at(2);
    const std::string& message = msg.at(3);

    std::map<std::string, ChannelInfo*>::iterator it(channelsJoined.find(channel));

    
    it->second->updateMemberInteractions(sender.substr(0, sender.find('!'))); //update the value of the interactions
    if (message.compare(0, 2, ":!") == 0)
    {
        if (message.compare(0, 18, ":!russianroulette") == 0)
        {
            if (channelsOperated.find(channel) != channelsOperated.end())
                enqueueMsg("KICK " + channel + " " + it->second->getRandomUser() + "\r\n");
            else
                enqueueMsg("PRIVMSG " + channel + " :" + "Sorry I am not a channel operator\r\n");
        }
        else if (message.compare(0, 11, ":!datetime") == 0)
            enqueueMsg("PRIVMSG " + channel + " :" + getTimeStamp() + "\r\n");
        else if (message.compare(0, 11, ":!lastseen") == 0)
            handleLastSeen(it->first, it->second, msg);
        else if (message.compare(0, 13, ":!mostactive") == 0)
            enqueueMsg("PRIVMSG " + channel + " :" 
                        + "Most active user : " + it->second->getMostActiveUser() + "\r\n"); 
    }
}

