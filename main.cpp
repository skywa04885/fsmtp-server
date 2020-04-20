/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <thread>
#include <map>
#include <cstring>

#include "src/server/server.src.hpp"
#include "src/ds-api/server.src.hpp"
#include "src/sender/sender.src.hpp"
#include "src/logger.src.hpp"

#include "src/mailer/mime-composer/mime-composer.src.hpp"

using namespace Fannst::FSMTPServer;

int main(int argc, char **argv) {

    Timer t("Composer");

    Mailer::Composer::MailerComposerOptions opts{};
    opts.o_PlainText = "Hello World";
    opts.o_HTML = "<h1>Hello World</h1>";
    opts.o_UseExistingSections = false;
    opts.o_To.emplace_back(Types::EmailAddress{"Test", "luke.rieff@gmail.com"});
    opts.o_From.emplace_back(Types::EmailAddress{"Webmaster", "webmaster@fannst.nl"});
    opts.o_Subject = "Hello World";
    opts.o_Domain = GE_DOMAIN;
    opts.o_DKIM.o_Domain = GE_DOMAIN;
    opts.o_DKIM.o_EnableDKIM = true;
    opts.o_DKIM.o_KeySelector = "default";
    opts.o_DKIM.o_PrivateKeyFile = "../keys/dkim/private-key.pem";

    char *message = nullptr;
    std::size_t messageLen;
    Mailer::Composer::sexyComposingAlgorithm(opts, &message, &messageLen);

    std::cout << "Message length:" << messageLen << std::endl;
    std::cout << "------" << std::endl;
    std::cout << message << std::endl;

    free(message);

    return 0;

    // Initializes the default values
    bool enableWebApi, enableSMTPServer, enableSender;
    enableWebApi = enableSMTPServer = false;

    // Creates the default logger
    Fannst::FSMTPServer::Logger::Console print(
            Fannst::FSMTPServer::Logger::Level::LOGGER_INFO, "Main");

    // Prints the initial message because it is under development
    print.setLevel(Fannst::FSMTPServer::Logger::Level::LOGGER_WARNING);
    print << "!! ALERT !! This application is highly under development, do not use for real world applications yet. "
        << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;
    print << "!! ALERT !! If you want to become an developer for Fannst Software, go to https://software.fannst.nl/contact "
          << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;
    print << "!! ALERT !! For any issues contact help@fannst.nl "
          << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;
    print.setLevel(Fannst::FSMTPServer::Logger::Level::LOGGER_INFO);

    // ----
    // Parses the command line arguments
    // ----

    if (argc == 1) enableSMTPServer = enableSender = true;
    else
    {
        print << "Arguments found, started parsing ..." << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;

        // Loops over the arguments
        char *arg = nullptr;
        std::size_t size;
        std::size_t i, j;
        char *t = reinterpret_cast<char *>(malloc(65));

        for (i = 1; i < argc; i++)
        {
            // Gets the argument
            arg = argv[i];
            // Gets the argument length
            size = strlen(&arg[0]);

            // Checks the type
            if (arg[0] == '-' && arg[1] == '-')
            { // It is an full command, like in words

                // Removes the dashes
                strcpy(&t[0], &arg[2]);

                // Get the index of the equals sign
                j = 0;
                for (char *p = &arg[2]; *p != '\0' && *p != '='; p++) j++;

                // Checks if the command contained an equals sign
                if (j == strlen(&arg[2]))
                {
                    print.setLevel(Fannst::FSMTPServer::Logger::Level::LOGGER_ERROR);
                    print << "Invalid command '" << arg << "', value is required at index: " << i
                          << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;
                    print.setLevel(Fannst::FSMTPServer::Logger::Level::LOGGER_ERROR);
                }

                // Checks if we have usage for the command
                if (t[0] == 'e' && strcmp(&t[0], "enable"))
                { // Starts with e

                    // Creates the tokenizer
                    char *tok = strtok(&t[7], ",");

                    // Loops over the args
                    while (tok != nullptr)
                    {
                        // Checks what the argument is
                        if (tok[0] == 'w' && strcmp(&tok[0], "web") == 0) enableWebApi = true;
                        else if (tok[0] == 's' && strcmp(&tok[0], "smtp") == 0) enableSMTPServer = true;
                        else if (tok[0] == 's' && strcmp(&tok[0], "sender") == 0) enableSender = true;

                        // Goes to the next token
                        tok = strtok(nullptr, ",");
                    }
                }
            } else
            { // Invalid command
                print.setLevel(Fannst::FSMTPServer::Logger::Level::LOGGER_ERROR);
                print << "Invalid command format '" << arg << "' at index: " << i
                    << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;
                print.setLevel(Fannst::FSMTPServer::Logger::Level::LOGGER_ERROR);
            }
        }

        // Free the memory
        free(t);
    }

    // ----
    // Runs the Web API
    // ----

    if (enableWebApi)
    {
        // Prints the update
        print << "Web API starting ..." << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;

        // Starts the web api
        Fannst::FSMTPServer::DS_API::run(80, &argc, &argv);
    }

    // ----
    // Runs the email sender server
    // ----

    if (enableSender)
    {
        // Prints the update
        print << "Sender starting ..." << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;

        // Creates the thread, if SMTP server enabled too
        if (!enableSMTPServer) Fannst::FSMTPServer::Sender::run(&argc, &argv);
        else
        {
            std::thread senderThread(Fannst::FSMTPServer::Sender::run, &argc, &argv);
            senderThread.detach();
        }
    }

    // ----
    // Runs the SMTP server
    // ----

    if (enableSMTPServer)
    {
        // Prints the update
        print << "FSMTP starting ..." << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL;

        // Creates the server
        Fannst::FSMTPServer::Server::run(1224, &argc, &argv);
    }

    // ----
    // Exits
    // ----

    return 0;
}
