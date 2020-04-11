#include <iostream>
#include <fstream>

#include <cassandra.h>

#include "src/server/server.src.hpp"
#include "src/email.src.hpp"
#include "src/logger.src.hpp"

int main() {
    //Runs the server
    server::run(25);
    // Closes with code 0
    return 0;
}
