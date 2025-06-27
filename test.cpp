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

    if (iter.at(3) == 'a')
        std::cout << "interesting;\n";
    for (std::string::iterator it = iter.begin(); it != iter.end(); ++it)
    {
        if (*it == '\r' && *(it + 1) == '\n')
            return (true);
    }

}