#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <set>

int main(void)
{
    std::string iter = "abba\r";

    for (std::string::iterator it = iter.begin(); it != iter.end(); ++it)
    {
        if (*it == '\r' && *(it + 1) == '\n')
            return (true);
    }

}