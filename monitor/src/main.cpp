#include "server.h"

#include <iostream>
#include <exception>


int main(int argc, char* argv[])
{
    try {
        Server server(5000);
        server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}