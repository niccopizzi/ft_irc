#include <string>
#include <vector>
#include <map>
#include <iostream>

int main(void)
{
    std::map<std::string, std::string&> test;
    

    std::string a("");
    std::string b("ciao");
    test.insert(std::pair<std::string, std::string&>("totest", b));

    test.erase("nonesiste");
    if (b.empty())
        std::cout << "gets removed\n";
    return (0);
    
}