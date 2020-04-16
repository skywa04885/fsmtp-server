/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "src/server/server.src.hpp"

int main(int argc, char **argv) {

    Fannst::FSMTPServer::Server::run(25, &argc, &argv);

    // ----
    // Exits
    // ----

    return 0;
}
