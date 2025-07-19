#include "ChannelInfo.hpp"

#include <iostream>


ChannelInfo::ChannelInfo()
{
    
}

ChannelInfo::ChannelInfo(const std::vector<std::string>& users)
{
    UserStat    userStat;
    std::vector<std::string>::const_iterator end = users.end();

    userStat.interactions = 0;
    userStat.lastSeen = std::time(NULL);
    for (std::vector<std::string>::const_iterator it = users.begin();
            it != end; ++it)
    {
        nickToStats.insert(std::make_pair(*it, userStat)); //init all users to last seen now and number of interactions 0
    }
}

ChannelInfo::ChannelInfo(const std::set<std::string>& users)
{
    UserStat    userStat;
    std::set<std::string>::const_iterator end = users.end();

    userStat.interactions = 0;
    userStat.lastSeen = std::time(NULL);
    for (std::set<std::string>::const_iterator it = users.begin();
        it != end; ++it)
    {
        nickToStats.insert(std::make_pair(*it, userStat)); //init all users to last seen now and number of interactions 0
    }
}

ChannelInfo::ChannelInfo(const ChannelInfo& chan) : nickToStats(chan.nickToStats)
{

}

ChannelInfo& ChannelInfo::operator=(const ChannelInfo& chan)
{
    if (this != &chan)
    {
        nickToStats = chan.nickToStats;
    }
    return (*this);
}

ChannelInfo::~ChannelInfo()
{

}

const std::map<std::string, UserStat>&  ChannelInfo::getStats() const
{
    return (nickToStats);
}

void ChannelInfo::updateMemberInteractions(const std::string& interactor)
{
    std::map<std::string, UserStat>::iterator it(nickToStats.find(interactor));

    if (it != nickToStats.end())
    {
        it->second.interactions += 1;
        it->second.lastSeen = std::time(NULL);
    }
}

void ChannelInfo::updateMemberLastSeen(const std::string& interactor)
{
    std::map<std::string, UserStat>::iterator it(nickToStats.find(interactor));

    if (it != nickToStats.end())
        it->second.lastSeen = std::time(NULL);
}

void ChannelInfo::updateMemberNick(const std::string& oldNick, const std::string& newNick)
{
    std::map<std::string, UserStat>::iterator it(nickToStats.find(oldNick));
    if (it == nickToStats.end())
        return;

    UserStat userstat;
    userstat.interactions = it->second.interactions;
    userstat.lastSeen = it->second.lastSeen;
    nickToStats.erase(it);
    nickToStats.insert(std::make_pair(newNick, userstat));
}

int ChannelInfo::getInteractions(const std::string& toSearch) const
{
    std::map<std::string, UserStat>::const_iterator it(nickToStats.find(toSearch));


    if (it != nickToStats.end())
        return (it->second.interactions);
    else
        return (-1);
}


time_t ChannelInfo::getLastSeen(const std::string& toSearch) const
{
    std::map<std::string, UserStat>::const_iterator it(nickToStats.find(toSearch));

    if (it != nickToStats.end())
        return (it->second.lastSeen);
    else
        return (0);
}

const std::string& ChannelInfo::getMostActiveUser() const
{
    std::map<std::string, UserStat>::const_iterator mostActive(nickToStats.begin());

    for (std::map<std::string, UserStat>::const_iterator it = nickToStats.begin();
        it != nickToStats.end(); ++it)
    {
        if (it->second.interactions > mostActive->second.interactions)
            mostActive = it;       
    }
    return (mostActive->first);
}

void ChannelInfo::addMember(const std::string& newMember)
{
    UserStat newMemberStats;
    std::map<std::string, UserStat>::iterator it(nickToStats.find(newMember));

    if (it != nickToStats.end())
    {
        it->second.lastSeen = std::time(NULL);
        return;
    }
    newMemberStats.interactions = 0;
    newMemberStats.lastSeen = std::time(NULL);
    nickToStats.insert(std::make_pair(newMember, newMemberStats));
}

const std::string& ChannelInfo::getRandomUser() const
{
    
    std::map<std::string, UserStat>::const_iterator it(nickToStats.begin());
    int randomPos = std::rand() % nickToStats.size();

    while (randomPos--)
    {
        ++it;
    }
    return (it->first);
}