#ifndef GANDHIBENDER_HPP
#define GANDHIBENDER_HPP

#include "Bender.hpp"

class GandhiBender : public Bender
{
    private:
        GandhiBender();
        GandhiBender(const GandhiBender& bbender);
        GandhiBender& operator=(const GandhiBender& bbender);

    public:
        GandhiBender(const std::string& password, const char* port);
        ~GandhiBender();

        void            handlePrivateMsg(const std::vector<std::string>& msg);
        void            handleChannelMsg(const std::vector<std::string>& msg);
        void            handleModeChange(const std::vector<std::string>& msg);
};

#endif