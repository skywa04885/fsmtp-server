#include <iostream>
#include <fstream>

#include <cassandra.h>

#include "src/server/server.src.hpp"
#include "src/email.src.hpp"
#include "src/logger.src.hpp"
#include "openssl/ssl.h"

int main(int argc, char **argv) {
    //Runs the server
    server::run(25, &argc, &argv);
    // Closes with code 0
    return 0;

}
