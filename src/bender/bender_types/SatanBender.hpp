#ifndef SATANBENDER_HPP
#define SATANBENDER_HPP

#include "../Bender.hpp"

class SatanBender : public Bender
{
    private:
        SatanBender();
        SatanBender(const SatanBender& bbender);
        SatanBender& operator=(const SatanBender& bbender);

        std::string genReplyMessage();

    public:
        SatanBender(const std::string& password, const char* port);
        ~SatanBender();

        void handleLastSeen(const std::string& chanName,
                            const ChannelInfo* chan,
                            const std::vector<std::string>& msg);
        void            handlePrivateMsg(const std::vector<std::string>& msg);
        void            handleChannelMsg(const std::vector<std::string>& msg);
        void            handleModeChange(const std::vector<std::string>& msg);
};

#endif