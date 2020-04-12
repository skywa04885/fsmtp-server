/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "server.src.hpp"
#include "./responses.src.hpp"

namespace server
{
    std::atomic<int> _usedThreads(0);
    int _maxThreads = MAX_THREADS;

    int run(const unsigned int& port)
    {
        // Prints text if debug enabled
        DEBUG_ONLY(std::cout << std::endl << "\033[1m\033[31m - SERVER DEBUG ENABLED - \033[0m" << std::endl << std::endl;);

        /*
         * Initializes the server socket
         */

        // Creates the logger
        DEBUG_ONLY(logger::Console print(logger::Level::LOGGER_INFO, "Run@Server"))
        DEBUG_ONLY(print << "Made by Luke Rieff ;)" << logger::ConsoleOptions::ENDL)

        // Creates the server struct
        struct sockaddr_in server{};
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        // Creates the socket
        int serverSock = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSock < 0)
        {
            DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_FATAL))
            DEBUG_ONLY(print << "Could not create socket, quitting !" << logger::ConsoleOptions::ENDL)
            return -1;
        }

        // Prints that the socket has been created
        DEBUG_ONLY(print << "Socket has been created." << logger::ConsoleOptions::ENDL)

        // Sets the socket options
        int option = 1;
        setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option));
        // Binds the socket
        if (bind(serverSock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            PREP_ERROR("Pre-Listening error", "Could not bind socket ... stopping !")
            return -1;
        }

        // Prints that the socket is bind
        DEBUG_ONLY(print << "Socket bind successfully." << logger::ConsoleOptions::ENDL);

        // Listens the server
        listen(serverSock, 40);

        // Prints that the socket is listening
        DEBUG_ONLY(print << "FSMTP Listening on port " << port << "." << logger::ConsoleOptions::ENDL)

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
            // Checks if there are threads ready for assignment
            if (_usedThreads > _maxThreads){
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
                continue;
            }

            // Accepts the client
            clientSocket = accept(serverSock, (struct sockaddr *)&client, (socklen_t *)&sockaddrSize);

            // Checks if the client was successfully accepted
            if (clientSocket < 0)
            {
                // Sets the print level to warning
                DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_WARNING))
                // Prints that the client could not connect successfully
                DEBUG_ONLY(print << "Client " << inet_ntoa(client.sin_addr) << " could not initialize connection." << logger::ConsoleOptions::ENDL)
                // Sets the print level to info
                DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_INFO))
                // Continues
                continue;
            }

            // Sets the socket timeout
            struct timeval timeout{};
            timeout.tv_sec = 9;
            timeout.tv_usec = 0;

            // Sets the socket timeout option
            if (
                    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 ||
                    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0
            ) {
                // Sets the print level to error
                DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_ERROR))
                // Prints the error
                DEBUG_ONLY(print << "Client " << inet_ntoa(client.sin_addr) <<
                    " could not be assigned an timeout, closing transmission channel .." <<
                    logger::ConsoleOptions::ENDL)
                // Sets the print level to info
                DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_INFO))
                // Closes the connection, and continues
                shutdown(clientSocket, SHUT_RDWR);
                continue;
            }

            // Assigns thread to client
            DEBUG_ONLY(print << "Client " << inet_ntoa(client.sin_addr) << " initialized connection, assigning thread .." <<
                logger::ConsoleOptions::ENDL)
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
        DEBUG_ONLY(print << "Transmitting message [ Message: " << message << " ]." << logger::ConsoleOptions::ENDL)
        // Appends the newline characters
        message.append("\r\n");
        // Sends the message
        if (send(*socket, message.c_str(), message.length(), 0) < 0)
        {
            // Sets the print level to error
            DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_ERROR))
            // Prints the error
            DEBUG_ONLY(print << "Could not send message [ Message: " << message << " ]." << logger::ConsoleOptions::ENDL)
            // Sets the print level to info
            DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_INFO))
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
        DEBUG_ONLY(print<< "Client made syntax error ! transmission channel closing, breaking." << logger::ConsoleOptions::ENDL)
    }

    void connectionThread(ConnectionThreadParams params)
    {
        // Claims one thread
        _usedThreads++;

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
        DEBUG_ONLY(print << "Thread assigned successfully." << logger::ConsoleOptions::ENDL)

        /**
         * Creates cassandra connection
         */

        bool connectionSuccess;
        cassandra::Connection connection("192.168.132.133", connectionSuccess);
        if (!connectionSuccess)
        {
            // Prints error message
            print.setLevel(logger::Level::LOGGER_FATAL);
            print << "Could not connect to cassandra, may not work properly." << logger::ConsoleOptions::ENDL;
            print.setLevel(logger::Level::LOGGER_INFO);
        } else
        {
            DEBUG_ONLY(print << "Thread successfully connected to cassandra." << logger::ConsoleOptions::ENDL)
        }

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
        std::string dataBuffer;
        // The received message body
        std::string sMessageBuffer;
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
            // Clears the buffer
            memset(buffer, 0, 1024);
            sBuffer.clear();
            // Reads the data
            readLen = recv(*params.clientSocket, buffer, sizeof(char) * 1024, 0);
            // Checks if there actually is data
            if (readLen <= 0)
            {
                // Sets the print level
                DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_WARNING))
                // Prints that the socket disconnected
                DEBUG_ONLY(print << "Client closed transmission channel, breaking." << logger::ConsoleOptions::ENDL)
                // Socket disconnected
                break;
            }
            // Assigns the sBuffer an string
            sBuffer = buffer;
            // Checks if data os currently being handled
            if (connPhasePt == ConnPhasePT::PHASE_PT_DATA)
            { // Should handle message body
                dataBuffer.append(sBuffer);
                // Checks if it contains the message end
                if (sBuffer.find("\r\n.\r\n") != std::string::npos)
                {
                    // Sets the status
                    connPhasePt = ConnPhasePT::PHASE_PT_DATA_END;

                    // Generates the response
                    response = serverCommand::generate(250, "Ok: message received ;)");
                    // Sends the response
                    sendMessage(params.clientSocket, response, print);

                    // Parses the message
                    parsers::parseMime(dataBuffer, result);

                    // Sets normally empty values
                    result.m_Timestamp = 0;

                    // Sets the timestamp of receive
                    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
                    result.m_ReceiveTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()).count();

                    // Creates an id for the message
                    CassUuidGen *uuidGen = cass_uuid_gen_new();
                    cass_uuid_gen_time(uuidGen, &result.m_UUID);
                    cass_uuid_gen_free(uuidGen);

                    result.save(connection.c_Session);

                    // Continues
                    continue;
                }
                // Continues
                continue;
            }
            // Parses the current command
            std::tie(currentCommand, currentCommandArgs) = serverCommand::parse(sBuffer);
            // Prints the current command
            DEBUG_ONLY(print << "Received [ Command: '" << serverCommand::serverCommandToString(currentCommand) << "', Argument: '" << currentCommandArgs << "' ]." << logger::ConsoleOptions::ENDL)
            // Checks how to respond
            switch (currentCommand)
            {
                // Client introduces
                case serverCommand::SMTPServerCommand::HELLO: {
                    responses::plain::handleHelo(params.clientSocket, currentCommandArgs, connPhasePt, params);
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
                // Mail from
                case serverCommand::SMTPServerCommand::MAIL_FROM: {
                    // Checks if command is allowed
                    if (connPhasePt >= ConnPhasePT::PHASE_PT_HELLO) {
                        // Parses the data
                        if (parsers::parseAddress(currentCommandArgs, result.m_TransportFrom) >= 0 && !currentCommandArgs.empty())
                        {
                            std::string username = result.m_TransportFrom.e_Address.substr(0, result.m_TransportFrom.e_Address.find_first_of("@"));
                            std::string domain = result.m_TransportFrom.e_Address.substr(result.m_TransportFrom.e_Address.find_first_of("@") + 1, result.m_TransportFrom.e_Address.length() - result.m_TransportFrom.e_Address.find_first_of("@") - 1);

                            // Checks if it is from an Fannst Sender, so the message needs to be added to relay queue
                            if (domain == "fannst.nl")
                            { // Is an sender from fannst
                                DEBUG_ONLY(print << "Message is being sent from Fannst Sender, possible relay accepted .." << logger::ConsoleOptions::ENDL)
                            }

                            // Generates the response
                            response = serverCommand::generate(250, "Ok Proceed");
                            // Sends the response
                            sendMessage(params.clientSocket, response, print);
                            // Sets the phase
                            connPhasePt = ConnPhasePT::PHASE_PT_MAIL_FROM;
                            // Prints an update
                            DEBUG_ONLY(print << "Message source: " << result.m_TransportFrom.e_Name << " <" << result.m_TransportFrom.e_Address << ">" << logger::ConsoleOptions::ENDL)
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
                        // Parses the data
                        if (parsers::parseAddress(currentCommandArgs, result.m_TransportTo) >= 0 && !currentCommandArgs.empty())
                        {
                            // Parses the username and domain name from email
                            std::string username = result.m_TransportTo.e_Address.substr(0, result.m_TransportTo.e_Address.find_first_of("@"));
                            std::string domain = result.m_TransportTo.e_Address.substr(result.m_TransportTo.e_Address.find_first_of("@") + 1, result.m_TransportTo.e_Address.length() - result.m_TransportTo.e_Address.find_first_of("@") - 1);

                            // Checks if the user is there
                            models::UserQuickAccess userQuickAccess;
                            int e = models::UserQuickAccess::selectByDomainAndUsername(connection.c_Session, domain.c_str(), username.c_str(), userQuickAccess);
                            if (e == -1)
                            {
                                // Generates the response
                                response = serverCommand::generate(
                                        471, "Something went wrong with Apache Cassandra");
                                // Sends the response
                                sendMessage(params.clientSocket, response, print);
                                // Ends
                                goto end;
                            } else if (e == -2)
                            {
                                // Generates the response
                                response = serverCommand::generate(551, "");
                                // Sends the response
                                sendMessage(params.clientSocket, response, print);
                                // Ends
                                goto end;
                            }

                            // Sets the user id to the email user id
                            result.m_UserUUID = userQuickAccess.u_Uuid;

                            // Generates the response
                            response = serverCommand::generate(250, "Ok Proceed");
                            // Sends the response
                            sendMessage(params.clientSocket, response, print);
                            // Sets the phase
                            connPhasePt = ConnPhasePT::PHASE_PT_MAIL_TO;
                            // Prints an update
                            DEBUG_ONLY(print << "Message target: " << result.m_TransportTo.e_Name << " <" <<
                                result.m_TransportTo.e_Address << ">" << logger::ConsoleOptions::ENDL)
                            // Breaks
                            break;
                        }
                        // Sends the syntax error
                        sendSyntaxError(params.clientSocket, "", print);
                        goto end;
                    }
                    // Sends the message that action is not allowed
                    sendInvalidOrderError(params.clientSocket, "MAIL FROM", print);
                    // Breaks
                    break;
                }
                // THe STARTTLS secion
                case serverCommand::SMTPServerCommand::START_TLS: {
                    break;
                }
                // The data section
                case serverCommand::SMTPServerCommand::DATA: {
                    // Checks if the command is allowed
                    if (connPhasePt >= ConnPhasePT::PHASE_PT_MAIL_TO)
                    {
                        // Generates the response
                        response = serverCommand::generate(354, "");
                        // Sends the response
                        sendMessage(params.clientSocket, response, print);
                        // Sets the phase
                        connPhasePt = ConnPhasePT::PHASE_PT_DATA;
                        // Breaks
                        break;
                    }
                    // Sends the message that action is not allowed
                    sendInvalidOrderError(params.clientSocket, "RCPT TO", print);
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
            DEBUG_ONLY(print.setLevel(logger::Level::LOGGER_WARNING))
            // Prints that the client disconnected
            DEBUG_ONLY(print << "Server closed transmission channel, client disconnected." << logger::ConsoleOptions::ENDL)

            // Makes thread available
            _usedThreads--;
    }
};