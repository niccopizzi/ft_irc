#ifndef GANDHIBENDER_HPP
#define GANDHIBENDER_HPP

#include "../Bender.hpp"

class GandhiBender : public Bender
{
    private:
        GandhiBender();
        GandhiBender(const GandhiBender& bbender);
        GandhiBender& operator=(const GandhiBender& bbender);

        std::string genReplyMessage();

    public:
        GandhiBender(const std::string& password, const char* port);
        ~GandhiBender();

        void            handleLastSeen(const std::string& chanName,
                            const ChannelInfo* chan,
                            const std::vector<std::string>& msg);

        void            kickall(const std::string& chanName, 
                                const ChannelInfo* chanToDestroy);

        void            handlePrivateMsg(const std::vector<std::string>& msg);
        void            handleChannelMsg(const std::vector<std::string>& msg);
        void            handleModeChange(const std::vector<std::string>& msg);
};

#endif