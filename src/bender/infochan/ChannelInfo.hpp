#ifndef CHANNELINFO_HPP
#define CHANNELINFO_HPP

#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <set>
#include <cstdlib>

struct UserStat
{
    int     interactions;
    time_t  lastSeen;
};

class ChannelInfo
{
private:
    ChannelInfo(const ChannelInfo& chan);
    ChannelInfo& operator=(const ChannelInfo& chan);
    std::map<std::string, UserStat> nickToStats;
    
public:
    ChannelInfo();
    ChannelInfo(const std::vector<std::string>& users);
    ChannelInfo(const std::set<std::string>& users);
    ~ChannelInfo();

    void addMember(const std::string& newMember);
    void updateMemberInteractions(const std::string& interactor);
    void updateMemberLastSeen(const std::string& interactor);
    void updateMemberNick(const std::string& oldNick, const std::string& newNick);

    const std::string& getMostActiveUser() const;
    const std::string& getRandomUser() const;

    int     getInteractions(const std::string& toSearch) const;
    time_t  getLastSeen(const std::string& toSearch) const;
    const std::map<std::string, UserStat>&    getStats() const;
    
};

#endif // ChannelInfo.HPP