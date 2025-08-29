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
    
    if (args.modeChanged == ":+o" && args.target == name)
    {
        channelsOperated.insert(args.channel);
        enqueueMsg("PRIVMSG " + args.channel + " :Powerrrr, this is what I was born for\r\n");
    }
    else if (args.modeChanged == ":-o" && args.target == name)
    {
        channelsOperated.erase(args.channel);
        enqueueMsg("PRIVMSG " + args.channel + " :You can slow me down, but you can't stop me.\r\n");
    }
    else
        enqueueMsg("PRIVMSG " + args.channel + 
                    " :STOP CHANGING MODES YOU STUP** MOT****[redacted]\r\n");
}

void SatanBender::handleLastSeen(const std::string& chanName,
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
        enqueueMsg("PRIVMSG " + chanName + " :How dare you try to break me, don't you know I am UNDEFEATABLE AHAHAHAHAH.\r\n");
    }
}

std::string SatanBender::genReplyMessage()
{
    static const char* theSubAndVerb[] = {"Your code", "Your life", "Your bedroom", "Your dream job", "Your stupid-ass dog"};
    static const char* theVerb = " is so";
    static const char* theAdj[] = { " shitty", " depressing", " spaghetti", " disgusting", " boring"};
    static const char* thePhrase[] = {" that a puppy died of trauma\r\n", " that the last person that saw it asked for a lobotomy\r\n", 
                                    " that I want to segfault and quit\r\n", " that a horse with no name ran away\r\n", 
                                    " that is not even worth commenting it\r\n"};

    std::string theReply(" :");

    theReply += theSubAndVerb[rand() % 5];
    theReply += theVerb;
    theReply += theAdj[rand() % 5];
    theReply += thePhrase[rand() % 5];

    return (theReply);
}

void SatanBender::handlePrivateMsg(const std::vector<std::string>& msg)
{
    std::string sender(msg.at(0).substr(1, msg.at(0).find('!') - 1));

    enqueueMsg("PRIVMSG " + sender + genReplyMessage());
}

void SatanBender::handleChannelMsg(const std::vector<std::string>& msg)
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
                enqueueMsg("KICK " + channel + " " + it->second->getRandomUser() + " :fuck you little shit!\r\n");
            else
                enqueueMsg("PRIVMSG " + channel + " :GIVE ME THE POWWWWEEEERR\r\n");
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