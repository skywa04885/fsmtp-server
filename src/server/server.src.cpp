#include "server.src.hpp"

namespace server
{
    int run(const unsigned int& port)
    {
        /*
         * Initializes the server socket
         */

        // Creates the logger
        logger::Console print(logger::Level::LOGGER_INFO, "Run@Server");
        // Creates the server struct
        struct sockaddr_in server{};
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        // Creates the socket
        int serverSock = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSock < 0)
        {
            print.setLevel(logger::Level::LOGGER_FATAL);
            print << "Could not create socket, quitting !" << logger::ConsoleOptions::ENDL;
            return -1;
        }
        // Prints that the socket has been created
        print << "Socket has been created." << logger::ConsoleOptions::ENDL;
        // Sets the socket options
        int option = 1;
        setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option));
        // Binds the socket
        if (bind(serverSock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            print.setLevel(logger::Level::LOGGER_FATAL);
            print << "Could not bind socket, quitting !" << logger::ConsoleOptions::ENDL;
            return -1;
        }
        // Prints that the socket is bind
        print << "Socket bind successfully." << logger::ConsoleOptions::ENDL;
        // Listens the server
        listen(serverSock, 40);
        // Prints that the socket is listening
        print << "FSMTP Listening on port " << port << "." << logger::ConsoleOptions::ENDL;

        /*
         * Starts accepting the clients
         */

        struct sockaddr_in client{};
        int sockaddrSize = sizeof(sockaddr_in);
        int clientSocket;
        // Starts the loop to accept
        // the clients
        for (;;)
        {
            // Accepts the client
            clientSocket = accept(serverSock, (struct sockaddr *)&client, (socklen_t *)&sockaddrSize);
            // Checks if the client was successfully accepted
            if (clientSocket < 0)
            {
                // Sets the print level to warning
                print.setLevel(logger::Level::LOGGER_WARNING);
                // Prints that the client could not connect successfully
                print << "Client " << inet_ntoa(client.sin_addr) << " could not initialize connection." << logger::ConsoleOptions::ENDL;
                // Sets the print level to info
                print.setLevel(logger::Level::LOGGER_INFO);
                // Continues
                continue;
            }
            // Sets the socket timeout
            struct timeval timeout{};
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            // Sets the socket timeout option
            if (
                    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 ||
                    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0
            ) {
                // Sets the print level to error
                print.setLevel(logger::Level::LOGGER_ERROR);
                // Prints the error
                print << "Client " << inet_ntoa(client.sin_addr) << " could not be assigned an timeout, closing transmission channel .." << logger::ConsoleOptions::ENDL;
                // Sets the print level to info
                print.setLevel(logger::Level::LOGGER_INFO);
                // Closes the connection, and continues
                shutdown(clientSocket, SHUT_RDWR);
                continue;
            }
            // Assigns thread to client
            print << "Client " << inet_ntoa(client.sin_addr) << " initialized connection, assigning thread .." << logger::ConsoleOptions::ENDL;
            // Copies the current variables
            // into a separate memory location
            int *clientSocketP = reinterpret_cast<int *>(malloc(sizeof(int)));
            memcpy(clientSocketP, &clientSocket, sizeof(int));
            auto *clientP = reinterpret_cast<struct sockaddr_in *>(malloc(sizeof(sockaddr_in)));
            memcpy(clientP, &client, sizeof(sockaddr_in));
            // Creates the thread parameters
            const ConnectionThreadParams params{
                clientSocketP,
                clientP
            };
            // Creates the thread
            std::thread thread(connectionThread, params);
            thread.detach();
        }
    }

    void sendMessage(const int *socket, std::string& message, logger::Console& print)
    {
        print << "Transmitting message [ Message: " << message << " ]." << logger::ConsoleOptions::ENDL;
        // Appends the newline characters
        message.append("\r\n");
        // Sends the message
        if (send(*socket, message.c_str(), message.length(), 0) < 0)
        {
            // Sets the print level to error
            print.setLevel(logger::Level::LOGGER_ERROR);
            // Prints the error
            print << "Could not send message [ Message: " << message << " ]." << logger::ConsoleOptions::ENDL;
            // Sets the print level to info
            print.setLevel(logger::Level::LOGGER_INFO);
        }
    }

    void sendInvalidOrderError(const int *socket, const char *param, logger::Console& print)
    {
        // Generates the response
        std::string response = serverCommand::generate(503, param);
        // Sends the response
        sendMessage(socket, response, print);
    }

    void sendSyntaxError(const int *socket, const char *param, logger::Console& print)
    {
        // Generates the response
        std::string response = serverCommand::generate(501, "");
        // Sends the response
        sendMessage(socket, response, print);
        // Shutdowns the socket
        print << "Client made syntax error ! transmission channel closing, breaking." << logger::ConsoleOptions::ENDL;
    }

    void connectionThread(ConnectionThreadParams params)
    {
        /**
         * Thread introduction
         */

        // Creates the logger string
        std::string printPrefix("(");
        printPrefix.append(inet_ntoa(params.client->sin_addr));
        printPrefix.append(") Client@Server");
        // Creates the logger
        logger::Console print(logger::Level::LOGGER_INFO, printPrefix.c_str());
        // Prints that the thread has been created
        print << "Thread assigned successfully." << logger::ConsoleOptions::ENDL;

        /**
         * The initial command line
         */

        // Generates the message
        std::string message = serverCommand::generate(220, "smtp.fannst.nl");
        // Sends the message
        sendMessage(params.clientSocket, message, print);

        /**
         * The read loop
         */

        // Creates the starting buffer, may
        // be resized later on
        char *buffer = reinterpret_cast<char *>(malloc(sizeof(char) * 1024));
        // The response
        std::string response;
        // The string variant of the buffer
        std::string sBuffer;
        // The arguments of the current command
        std::string currentCommandArgs;
        // The current command
        serverCommand::SMTPServerCommand currentCommand;
        // The current phase
        ConnPhasePT connPhasePt = ConnPhasePT::PHASE_PT_INITIAL;
        // The current buffer read length
        int readLen;
        // The result email
        models::Email result;
        // The read loop
        for (;;)
        {
            // Reads the data
            readLen = recv(*params.clientSocket, buffer, sizeof(char) * 1024, 0);
            // Checks if there actually is data
            if (readLen <= 0)
            {
                // Sets the print level
                print.setLevel(logger::Level::LOGGER_WARNING);
                // Prints that the socket disconnected
                print << "Client closed transmission channel, breaking." << logger::ConsoleOptions::ENDL;
                // Socket disconnected
                break;
            }
            // Assigns the sBuffer an string
            sBuffer = buffer;
            // Parses the current command
            std::tie(currentCommand, currentCommandArgs) = serverCommand::parse(sBuffer);
            // Prints the current command
            print << "Received [ Command: '" << serverCommand::serverCommandToString(currentCommand) << "', Argument: '" << currentCommandArgs << "' ]." << logger::ConsoleOptions::ENDL;
            // Checks how to respond
            switch (currentCommand)
            {
                // Client introduces
                case serverCommand::SMTPServerCommand::HELLO: {
                    // Checks if action is valid
                    if (connPhasePt >= ConnPhasePT::PHASE_PT_INITIAL)
                    {
                        // Generates the response
                        std::string temp = std::string(inet_ntoa(params.client->sin_addr)) + " nice to meet you!";
                        response = serverCommand::generate(250, temp.c_str());
                        // Sends the response
                        sendMessage(params.clientSocket, response, print);
                        // Updates the phase
                        connPhasePt = ConnPhasePT::PHASE_PT_HELLO;
                        // Breaks
                        break;
                    }
                    // Sends the message that action is not allowed
                    sendInvalidOrderError(params.clientSocket, "idk, initialize ?", print);
                    // Breaks
                    break;
                }
                // Client requests exit
                case serverCommand::SMTPServerCommand::QUIT: {
                    // Generates the response
                    response = serverCommand::generate(221, "");
                    // Sends the response
                    sendMessage(params.clientSocket, response, print);
                    // Shutdowns the socket
                    print << "Client requested transmission channel closing, breaking." << logger::ConsoleOptions::ENDL;
                    // goes to the end
                    goto end;
                }
                // MAIL FROM:Luke Rieff <luke.rieff@gmail.com>
                // Mail from
                case serverCommand::SMTPServerCommand::MAIL_FROM: {
                    // Checks if command is allowed
                    if (connPhasePt >= ConnPhasePT::PHASE_PT_HELLO) {
                        // Parses the data
                        if (models::parsers::parseAddress(currentCommandArgs, result.m_TransportFrom) >= 0 && !currentCommandArgs.empty())
                        {
                            // Generates the response
                            response = serverCommand::generate(250, "OK Proceed");
                            // Sends the response
                            sendMessage(params.clientSocket, response, print);
                            // Sets the phase
                            connPhasePt = ConnPhasePT::PHASE_PT_MAIL_FROM;
                            std::cout << result.m_TransportFrom.e_Name << "," << result.m_TransportFrom.e_Address << std::endl;
                            // Breaks
                            break;
                        }
                        // Sends the syntax error
                        sendSyntaxError(params.clientSocket, "", print);
                        goto end;
                    } else {
                        // Sends the message that action is not allowed
                        sendInvalidOrderError(params.clientSocket, "HELO", print);
                        // Breaks
                        break;
                    }
                }
                // RCPT To
                case serverCommand::SMTPServerCommand::RCPT_TO: {
                    // Checks if command is allowed
                    if (connPhasePt >= ConnPhasePT::PHASE_PT_MAIL_FROM) {
                        // Generates the response
                        response = serverCommand::generate(250, "OK Proceed");
                        // Sends the response
                        sendMessage(params.clientSocket, response, print);
                        // Sets the phase
                        connPhasePt = ConnPhasePT::PHASE_PT_MAIL_TO;
                        // Breaks
                        break;
                    }
                    // Sends the message that action is not allowed
                    sendInvalidOrderError(params.clientSocket, "MAIL FROM", print);
                    // Breaks
                    break;
                }
                // Command not found
                default: {
                    sendSyntaxError(params.clientSocket, "", print);
                    goto end;
                }
            }
        }
        // The label for the end
        end:
            // Closes the socket
            shutdown(*params.clientSocket, SHUT_RDWR);
            // Deletes the params,
            // to avoid memory leaks
            delete params.clientSocket;
            delete params.client;
            // Sets the print level
            print.setLevel(logger::Level::LOGGER_WARNING);
            // Prints that the client disconnected
            print << "Server closed transmission channel, client disconnected." << logger::ConsoleOptions::ENDL;
    }
};