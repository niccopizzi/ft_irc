#ifndef IRC_HPP
#define IRC_HPP

#include <iostream>
#include "server/Server.hpp"
#include "signals/signals.hpp"

bool    areArgsValid(int argc, char* argv[]);
void    startServer(char* port, char* password);
#endif