#include "Operator.hpp"
//#include "User.hpp"

User::User() :  nickname(""),
                username(""),
                fullname(""),
                didAuthenticate(false)
{
    oper = NULL;
    #ifdef DEBUG
        std::cout << "User default constructor called\n";
    #endif
}

User::User(const std::string& nick, const std::string& uname) : nickname(nick),
                                                                username(uname),
                                                                fullname(""),
                                                                didAuthenticate(false)

{
    oper = NULL;
    #ifdef DEBUG
        std::cout << "User constructor called\n";
    #endif
}

User::User(const User& user) :  nickname(user.nickname),
                                username(user.username),
                                fullname(user.fullname),
                                oper(user.oper),
                                didAuthenticate(user.didAuthenticate)
{
    #ifdef DEBUG
        std::cout << "User copy constructor called\n";
    #endif
}
User& User::operator=(const User& other)
{
    if (this != &other)
    {
        nickname = other.username;
        username = other.username;
        fullname = other.fullname;
        didAuthenticate = other.didAuthenticate;
        if (oper != NULL)
            delete oper;
        oper = other.oper;
    }
    #ifdef DEBUG
        std::cout << "User copy operator called\n";
    #endif
    return (*this);
}

User::~User()
{
    nickname.clear();
    username.clear();
    fullname.clear();
    didAuthenticate = false;
    if (oper != NULL)
        delete oper;
    #ifdef DEBUG
        std::cout << "User destructor called\n";
    #endif
}

const std::string& User::getNickname() const
{
    return (nickname);
}

const std::string& User::getUsername() const
{
    return (username);
}

const std::string& User::getFullname() const
{
    return (fullname);
}

bool    User::isAuthenticated() const
{
    return (didAuthenticate);
}

bool    User::isRegistered() const
{
    return (nickname.empty() == false && 
            username.empty() == false);
}

void    User::setNickname(const std::string& newNick)
{
    nickname = newNick;
}

void    User::setUsername(const std::string& newUsername)
{
    username = newUsername; 
}

void    User::setFullname(const std::string& newFullname)
{
    fullname = newFullname;
}

void    User::setAuthenticate(bool auth)
{
    didAuthenticate = auth;
}

void    User::becomeOperator()
{
    oper = new Operator();
}