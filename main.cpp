/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "src/server/server.src.hpp"

int main(int argc, char **argv) {
    // ----
    // Runs the server
    // ----

    fcb a = fcb_new("test");
    fcb b = fcb_new("-test12");

    fcb_resize(&a, 6);
    fcb_strcat(a, 4);

    std::cout << a << ", " << fcb_strlen(a) << "," << fcb_findchar(a, ':') << std::endl;

    fcb_free(a);
    fcb_free(b);

//    Fannst::FSMTPServer::Server::run(1125, &argc, &argv);

    // ----
    // Exits
    // ----

    return 0;
}
