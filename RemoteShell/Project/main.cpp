//
// Created by roman sztergbaum on 04/10/2017.
//

#include <exception>
#include <iostream>
#include "Client.hpp"

int main()
{
    try {
        sh::Client shell;

        shell.run();
    }
    catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
    }
    return 0;
}