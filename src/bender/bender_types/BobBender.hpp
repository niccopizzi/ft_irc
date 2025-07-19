#ifndef BOBBENDER_HPP
#define BOBBENDER_HPP

#include "../Bender.hpp"

class BobBender : public Bender
{
    private:
        BobBender();
        BobBender(const BobBender& bbender);
        BobBender& operator=(const BobBender& bbender);

    public:
        BobBender(const std::string& password, const char* port);
        ~BobBender();

        void            handleLastSeen(const std::string& chanName,
                                    const ChannelInfo* chan,
                                    const std::vector<std::string>& msg);
        void            handlePrivateMsg(const std::vector<std::string>& msg);
        void            handleChannelMsg(const std::vector<std::string>& msg);
        void            handleModeChange(const std::vector<std::string>& msg);
};

#endif