#include <iostream>
#include <fstream>

#include <cassandra.h>

#include "src/server/server.src.hpp"
#include "src/email.src.hpp"
#include "src/logger.src.hpp"
#include "openssl/ssl.h"
#include "src/server/commands.src.hpp"
#include "src/parsers/mail-parser.src.hpp"
#include "src/mailer/mailer.src.hpp"

int main(int argc, char **argv) {
    //Runs the server
//    Fannst::FSMTPServer::Server::run(25, &argc, &argv);
    fannst::composer::Options options;
    options.o_HTML = "<h1>Hello World</h1>";
    options.o_Text = "Hello World";
    options.o_From.emplace_back("Luke Rieff", "luke.rieff@fannst.nl");
    options.o_To.emplace_back("Luke A.C.A. Rieff", "jrieff@notariskantoorbergen.nl");
    options.o_Subject = "Hello World Test";
    options.o_Domain = "fannst.nl";

    Fannst::Mailer mailer(options);

    if (mailer.sendMessage() < 0)
    {
        std::cout << "Send failed" << std::endl;
    } else {
        std::cout << "Send Complete!" << std::endl;
    }
    // Closes with code 0
    return 0;

}
