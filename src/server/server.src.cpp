/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <fcntl.h>
#include "server.src.hpp"
#include "./responses.src.hpp"

namespace server
{
    std::atomic<int> _usedThreads(0);
    int _maxThreads = MAX_THREADS;

    /**
     * Runs an server instance
     * @param port
     * @return
     */
    int run(const unsigned int& port, int *argc, char ***argv)
    {
        // Prints text if debug enabled
        DEBUG_ONLY(std::cout << std::endl << "\033[1m\033[31m - SERVER DEBUG ENABLED - \033[0m" << std::endl << std::endl;);

        // ----
        // Initializes required modules
        // ----

        OPENSSL_init();

        // ----
        // Initializes the server socket
        // ----

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
            PREP_ERROR("Could not create socket", "We do not know why .")
            return -1;
        }

        // Sets the server to be non-blocking
        fcntl(serverSock, F_SETFL, O_NONBLOCK);

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

        // Listens the server
        listen(serverSock, 40);

        // Prints that the socket is listening
        DEBUG_ONLY(print << "FSMTP Listening on port " << port << "." << logger::ConsoleOptions::ENDL)

        // ----
        // Defines the required structures and sizes
        // ----

        int sockaddrSize = sizeof(sockaddr_in);
        struct sockaddr_in client{};
        int clientSocket;

        // ----
        // Infinite loop to accept clients
        // ----

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
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
                PREP_ERROR("Could not set option", "SOL_SOCKET, SO_RCVTIMEO");
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

    /**
     * Reads passphrase file for OpenSSL Sockets
     * @param buffer
     * @param size
     * @param rwflag
     * @param u
     * @return
     */
    int sslConfigureContexLoadPassword(char *buffer, int size, int rwflag, void *u)
    {
        // Opens the file, with reading mode
        FILE *file = fopen("../keys/ssl/pp.txt", "r");
        if (file == nullptr)
        {
            // TODO: Handle file error
        }

        // Reads the password
        fgets(&buffer[0], size, file);
        // Returns the length
        return strlen(&buffer[0]);
    }

    /**
     * Configures an OpenSSL context for OpenSSL Sockets
     * @param ctx
     * @return
     */
    int sslConfigureContext(SSL_CTX *ctx)
    {
        SSL_CTX_set_ecdh_auto(ctx, 1);
        SSL_CTX_set_default_passwd_cb(ctx, &sslConfigureContexLoadPassword);

        // Sets the certificate
        if (SSL_CTX_use_certificate_file(ctx, "../keys/ssl/cert.pem", SSL_FILETYPE_PEM) <= 0)
        {
            ERR_print_errors_fp(stderr);
            return -1;
        }

        // Sets the private key
        if (SSL_CTX_use_PrivateKey_file(ctx, "../keys/ssl/key.pem", SSL_FILETYPE_PEM) <= 0)
        {
            ERR_print_errors_fp(stderr);
            return -1;
        }

        return 0;
    }

    /**
     * Method used for single thread
     * @param params
     */
    void connectionThread(ConnectionThreadParams params)
    {
        _usedThreads++;             // Reserves one thread, in global atomic

        // ----
        // The startup of the thread
        // ----

        #ifdef DEBUG
        // Creates the logger prefix, with the remote address
        std::string printPrefix("(");
        printPrefix.append(inet_ntoa(params.client->sin_addr));
        printPrefix.append(") Client@Server");

        // Prints the result
        logger::Console print(logger::Level::LOGGER_INFO, printPrefix.c_str());
        print << "Thread assigned successfully." << logger::ConsoleOptions::ENDL;
        #endif

        // ----
        // Connects to cassandra
        // ----

        bool rb;
        cassandra::Connection connection(GE_CASSANDRA_CONTACT_POINTS, rb);

        // Checks if the connection went successfully
        if (!rb)
        {
            #ifdef DEBUG
            print.setLevel(logger::Level::LOGGER_FATAL);
            print << "Could not connect to cassandra, may not work properly." << logger::ConsoleOptions::ENDL;
            print.setLevel(logger::Level::LOGGER_INFO);
            #endif
        } else
        {
            DEBUG_ONLY(print << "Thread connected to DataStax Cassandra" << logger::ConsoleOptions::ENDL)
        }

        // ----
        // Sends the initial greeting message
        // ----

        {
            // Generates the message
            const char *message = serverCommand::gen(220, "smtp.fannst.nl");

            // Writes the message
            responses::plain::write(params.clientSocket, message, strlen(message));

        }

        // ----
        // The current state
        // ----

        ConnPhasePT connPhasePt = ConnPhasePT::PHASE_PT_INITIAL;

        // ----
        // The data storage, buffers etc
        // ----

        serverCommand::SMTPServerCommand currentCommand;
        std::string currentCommandArgs;
        std::string sMessageBuffer;
        std::string dataBuffer;
        std::string response;
        std::string sBuffer;

        char buffer[1024];          // Kinda large buffer for storing commands, and message temp
        int readLen = 0;            // The amount of chars, read in the thread at specific moment

        models::Email result;       // The result email

        // ----
        // The connection status variables
        // ----

        bool err = false;           // If the current thread has errored out
        bool usingSSL = false;      // If the connection is making use of START TLS

        // ----
        // The OpenSSL Connection variables
        // ----

        SSL_CTX *sslCtx = nullptr;
        SSL *ssl = nullptr;

        // ----
        // Infinite loop, used to process data
        // ----

        for (;;)
        {
            // Clears the buffer
            memset(buffer, 0, sizeof(buffer));
            sBuffer.clear();

            // Waits for data to be transmitted
            readLen = recv(*params.clientSocket, buffer, sizeof(char) * 1024, 0);

            // Checks if there was data received
            if (readLen <= 0)
            {
                #ifdef DEBUG
                print.setLevel(logger::Level::LOGGER_WARNING);
                print << "Client closed transmission channel, breaking." << logger::ConsoleOptions::ENDL;
                #endif

                // Closes connection
                goto end;
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

                    // Saves the email
//                    result.save(connection.c_Session);
                    std::cout << result << std::endl;

                    // Continues
                    continue;
                }
                // Continues
                continue;
            }

            // Parses the current command
            std::tie(currentCommand, currentCommandArgs) = serverCommand::parse(sBuffer);

            // ----
            // Checks how the server should respond, and process the operation
            // ----

            switch (currentCommand)
            {
                // Client introduces
                case serverCommand::SMTPServerCommand::HELLO: {
                    if (!responses::plain::handleHelo(params.clientSocket, currentCommandArgs,
                            connPhasePt, params)) {
                        err = true;
                        goto end;
                    };
                    break;
                }

                // Client requests exit
                case serverCommand::SMTPServerCommand::QUIT: {
                    responses::plain::handleQuit(params.clientSocket);
                    goto end;
                }

                // Handles 'MAIL_FROM'
                case serverCommand::SMTPServerCommand::MAIL_FROM: {
                    if (!responses::plain::handleMailFrom(params.clientSocket, currentCommandArgs,
                            result, connPhasePt)) {
                        err = true;
                        goto end;
                    };
                    break;
                }

                // Handles 'RCPT TO'
                case serverCommand::SMTPServerCommand::RCPT_TO: {
                    if (!responses::plain::handleRcptTo(params.clientSocket, currentCommandArgs, result,
                            connPhasePt, connection.c_Session)) {
                        err = true;
                        goto end;
                    };
                    break;
                }

                // Handles 'START TLS'
                case serverCommand::SMTPServerCommand::START_TLS: {
                    std::cout << "Test" << std::endl;
                    // Writes the message
                    const char *message = serverCommand::gen(220, "Go ahead");
                    responses::plain::write(params.clientSocket, message, strlen(message));

                    // ----
                    // Initializes OpenSSL
                    // ----

                    // Creates the context
                    const SSL_METHOD *method = SSLv23_server_method();
                    sslCtx = SSL_CTX_new(method);
                    if (!sslCtx)
                    {
                        PREP_ERROR("SSL Error", "Could not create context ..")
                        ERR_print_errors_fp(stderr);
                        // TODO: Send error
                    }

                    // Assigns the keys and certificate
                    if (sslConfigureContext(sslCtx) < 0)
                    {
                        // TODO: Send error
                    }

                    // ----
                    // Uses OpenSSL
                    // ----

                    // Creates the ssl
                    ssl = SSL_new(sslCtx);

                    // Sets the socket file descriptor
                    SSL_set_fd(ssl, *params.clientSocket);

                    // Starts the SSL connection
                    if (SSL_accept(ssl) <= 0)
                    {
                        PREP_ERROR("OpenSSL Error", "Could not accept client ..")
                        ERR_print_errors_fp(stderr);
                    }

                    // Resets the state
                    connPhasePt = ConnPhasePT::PHASE_PT_INITIAL;

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
                    responses::plain::syntaxError(params.clientSocket);
                    err = true;
                    goto end;
                }
            }
        }

        // ----
        // End section, will shut down the socket, and perform other stopping operations
        // ----

        end:
            // Closes the socket
            shutdown(*params.clientSocket, SHUT_RDWR);

            // Deletes the params,
            // to avoid memory leaks
            delete params.clientSocket;
            delete params.client;

            // Makes thread available
            _usedThreads--;

            #ifdef DEBUG
            print.setLevel(logger::Level::LOGGER_WARNING);
            print << "Server closed transmission channel, client disconnected." << logger::ConsoleOptions::ENDL;
            #endif
    }
};