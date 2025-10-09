#ifndef SIGNALS_HPP
#define SIGNALS_HPP

#include <signal.h>
#include <iostream>

extern volatile sig_atomic_t    serverShouldRun;

void registerSignalHandlers();

#endif