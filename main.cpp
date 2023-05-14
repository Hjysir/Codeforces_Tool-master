#include <iostream>
#include "CodeforcesAPI.h"

int main()
{
    std::ios::sync_with_stdio(false);
    CodeforcesAPI api;
    std::cout << "Name :";
    std::cin >> api.Username;
    CodeforcesAPI::Get(api.Username);
    api.PrintData();
    return 0;
}
