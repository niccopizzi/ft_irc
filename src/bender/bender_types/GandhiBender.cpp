#include "GandhiBender.hpp"

GandhiBender::GandhiBender(const std::string& pass, const char* port) : Bender(pass, "GandhiBender", port)
{
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
        enqueueMsg("PRIVMSG " + args.channel + 
                " :How heavy is the toll of sins and wrong that wealth, power and prestige exact from man.\r\n");
    }
    else if (args.modeChanged == ":-o" && args.target == name)
    {
        channelsOperated.erase(args.channel);
        enqueueMsg("PRIVMSG " + args.channel + " :An eye for an eye qill only make the whole world blind.\r\n");
    }
    else
        enqueueMsg("PRIVMSG " + args.channel + 
            " :Freedom is not worth having if it does not include the freedom to make mistakes.\r\n");
}

std::string GandhiBender::genReplyMessage()
{
    static const char* theSub[] = {"Freedom", "A man", "God", "The future", "Happiness"};
    static const char* theVerb[] = {" is", " likes", " has"};
    static const char* theObj[] = {" an indomitable will\r\n", " a horse with no name\r\n", 
                                    " a drop in the ocean\r\n", " anything but ft_irc\r\n"};

    std::string theReply(" :");

    theReply += theSub[rand() % 5];
    theReply += theVerb[rand() % 3];
    theReply += theObj[rand() % 4];

    return (theReply);
}

void GandhiBender::handlePrivateMsg(const std::vector<std::string>& msg)
{
    std::string sender(msg.at(0).substr(1, msg.at(0).find('!') - 1));

    enqueueMsg("PRIVMSG " + sender + genReplyMessage());
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
            enqueueMsg("PRIVMSG " + chanName + " :" + msg.at(4) + " :Never seen on this channel\r\n");
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
        if (it->first != this->name)
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
        if (message.compare(0, 12, ":!underflow") == 0 &&
            channelsOperated.find(channel) != channelsOperated.end())
        {
            enqueueMsg("PRIVMSG " + channel + " :Secret mode unlocked: CHAOS ENSUES\r\n");
            kickall(channel, it->second);
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

