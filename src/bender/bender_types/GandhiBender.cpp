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
    
    if (args.modeChanged == ":+o" && args.target == name)
    {
        channelsOperated.insert(args.channel);
    }
    else if (args.modeChanged == ":-o" && args.target == name)
    {
        channelsOperated.erase(args.channel);
    }
    else
        enqueueMsg("PRIVMSG " + args.channel + 
            " Freedom is not worth having if it does not include the freedom to make mistakes.\r\n");
}

void GandhiBender::handlePrivateMsg(const std::vector<std::string>& msg)
{

    std::string sender(msg.at(0).substr(1, msg.at(0).find('!') - 1));

    enqueueMsg("PRIVMSG " + sender + " :Ciao!\r\n");

}

void GandhiBender::handleLastSeen(const std::string& chanName,
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
        enqueueMsg("PRIVMSG " + chanName + " :A man is but the product of his thoughts. What he thinks, he becomes.\r\n");
    }
}

int clamp(int toClamp, int min, int max)
{
    return (std::max(min, std::min(toClamp, max)));
}

std::string genRandKey()
{
    std::string randKey;

    for (int i = 0; i < 10; ++i)
        randKey.push_back((char)(clamp(std::rand() % 127, 32, 126)));
    return (randKey);
}

void GandhiBender::kickall(const std::string& chanName, const ChannelInfo* chanToDestroy)
{
    const std::string incipit("KICK " + chanName + " ");

    for (std::map<std::string, UserStat>::const_iterator it = chanToDestroy->getStats().begin();
    it != chanToDestroy->getStats().end(); ++it)
    {
        if (it->first != name)
            enqueueMsg(incipit + it->first + "\r\n");
    }
    enqueueMsg("MODE " + chanName + " +k+l " + genRandKey() + " 1\r\n");
}

void GandhiBender::handleChannelMsg(const std::vector<std::string>& msg)
{
    const std::string& sender = msg.at(0).substr(1);
    const std::string& channel = msg.at(2);
    const std::string& message = msg.at(3);

    std::map<std::string, ChannelInfo*>::iterator it(channelsJoined.find(channel));

    
    it->second->updateMemberInteractions(sender.substr(0, sender.find('!'))); //update the value of the interactions
    if (message.compare(0, 2, ":!") == 0)
    {
        if (message.compare(0, 12, ":!underflow") == 0)
        {
            if (channelsOperated.find(channel) != channelsOperated.end())
            {
                enqueueMsg("PRIVMSG " + channel + " :Secret mode unlocked: CHAOS ENSUES\r\n");
                kickall(channel, it->second);
            }
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

