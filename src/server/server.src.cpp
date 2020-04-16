/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <fannst_libcompose.hpp>

#include "server.src.hpp"
#include "./responses.src.hpp"

namespace Fannst::FSMTPServer::Server
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

        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        // ----
        // Initializes the server socket
        // ----

        // Creates the logger
        DEBUG_ONLY(Logger::Console print(Logger::Level::LOGGER_INFO, "Run@Server"))
        DEBUG_ONLY(print << "Made by Luke Rieff ;)" << Logger::ConsoleOptions::ENDL)

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

        // Prints that the socket has been created
        DEBUG_ONLY(print << "Socket has been created." << Logger::ConsoleOptions::ENDL)

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
        DEBUG_ONLY(print << "FSMTP Listening on port " << port << "." << Logger::ConsoleOptions::ENDL)

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

            // Accepts the mailer
            clientSocket = accept(serverSock, (struct sockaddr *)&client, (socklen_t *)&sockaddrSize);

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

            // Assigns thread to mailer
            DEBUG_ONLY(print << "Client " << inet_ntoa(client.sin_addr) << " initialized connection, assigning thread .." << Logger::ConsoleOptions::ENDL)
            // Copies the current variables
            // into a separate memory location
            auto *clientP = reinterpret_cast<struct sockaddr_in *>(malloc(sizeof(sockaddr_in)));
            memcpy(clientP, &client, sizeof(sockaddr_in));

            // Creates the thread
            std::thread thread(connectionThread, clientP, clientSocket);
            thread.detach();
        }

        // ----
        // Cleans up the stuff
        // ----

        EVP_cleanup();
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
        FILE *file = fopen("../keys/ssl/pp.pem", "r");
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
    void connectionThread(struct sockaddr_in *sockaddrIn, int sock_fd)
    {
        _usedThreads++;             // Reserves one thread, in global atomic

        // ----
        // The startup of the thread
        // ----

        #ifdef DEBUG
        // Creates the logger prefix, with the remote address
        std::string printPrefix("(");
        printPrefix.append(inet_ntoa(sockaddrIn->sin_addr));
        printPrefix.append(") Client@Server");

        // Prints the result
        Logger::Console print(Logger::Level::LOGGER_INFO, printPrefix.c_str());
        print << "Thread assigned successfully." << Logger::ConsoleOptions::ENDL;
        #endif

        // ----
        // Connects to cassandra
        // ----

        bool rb;
        Cassandra::Connection connection(GE_CASSANDRA_CONTACT_POINTS, rb);

        // Checks if the connection went successfully
        if (!rb)
        {
            #ifdef DEBUG
            print.setLevel(Logger::Level::LOGGER_FATAL);
            print << "Could not connect to cassandra, may not work properly." << Logger::ConsoleOptions::ENDL;
            print.setLevel(Logger::Level::LOGGER_INFO);
            #endif
        } else
        {
            DEBUG_ONLY(print << "Thread connected to DataStax Cassandra" << Logger::ConsoleOptions::ENDL)
        }

        // ----
        // Sends the initial greeting message
        // ----

        {
            // Generates the message
            char *message = ServerCommand::gen(220, "smtp.fannst.nl - ESMTP Ready", nullptr, 0);

            // Writes the message
            Responses::write(&sock_fd, nullptr, message, strlen(&message[0]));

            // Deletes the message
            free(message);
        }

        // ----
        // The current state
        // ----

        ConnPhasePT connPhasePt = ConnPhasePT::PHASE_PT_INITIAL;

        // ----
        // The data storage, buffers etc
        // ----

        ServerCommand::SMTPServerCommand currentCommand;
        const char *currentCommandArgs;
        std::string sMessageBuffer;
        std::string dataBuffer;
        std::string response;
        std::string sBuffer;

        char buffer[1024];          // Kinda large buffer for storing commands, and message temp
        int readLen = 0;            // The amount of chars, read in the thread at specific moment

        Models::Email result;       // The result email

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

            // ----
            // Prepares the buffers
            // ----

            memset(buffer, 0, sizeof(buffer));
            sBuffer.clear();


            // ----
            // Reads the data
            // ----

            if (!usingSSL) readLen = recv(sock_fd, &buffer[0], 1024, 0);
            else readLen = SSL_read(ssl, &buffer[0], 1024);


            // ----
            // Checks if there went something wrong
            // ----

            if (readLen <= 0)
            {
                #ifdef DEBUG
                print.setLevel(Logger::Level::LOGGER_WARNING);
                print << "Client closed transmission channel, breaking." << Logger::ConsoleOptions::ENDL;
                #endif

                // Closes connection
                goto end;
            }

            // Assigns the sBuffer an string
            sBuffer = buffer;

            // ----
            // Checks if there is data being handled, if so append to buffer
            // ----

            if (connPhasePt == ConnPhasePT::PHASE_PT_DATA)
            { // Should handle message body
                dataBuffer.append(sBuffer);
                // Checks if it contains the message end
                std::size_t id = std::string::npos;
                if (dataBuffer.length() > 11)
                {
                    id = dataBuffer.substr(dataBuffer.length() - 10, dataBuffer.length()).find("\r\n.\r\n");
                }
                if (id != std::string::npos)
                {
                    Fannst::Composer::prettyPrintBody(dataBuffer);

                    // Sets the status
                    connPhasePt = ConnPhasePT::PHASE_PT_DATA_END;

                    // Sends the response
                    const char *message = ServerCommand::gen(250, "OK: Message received by Fannst", nullptr, 0);
                    Responses::write(&sock_fd, ssl, message, strlen(message));
                    delete message;

                    // Parses the message
                    Parsers::parseMime(dataBuffer, result);

                    // Sets normally empty values
                    result.m_Timestamp = 0;

                    // Sets the timestamp of receive
                    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
                    result.m_ReceiveTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()).count();

                    // ----
                    // Finishes the email, and stores it
                    // ----

                    // Generates an UUID for the email
                    CassUuidGen *uuidGen = cass_uuid_gen_new();
                    cass_uuid_gen_time(uuidGen, &result.m_UUID);
                    cass_uuid_gen_free(uuidGen);

                    // Sets the receive params
                    result.m_FullHeaders.push_back(Models::EmailHeader{"X-FN-TransportType", usingSSL ? "START TLS" : "Plain Text"});
                    result.m_FullHeaders.push_back(Models::EmailHeader{"X-FN-ServerVersion", "FSMTP 1.0"});

                    // Saves the email
                    result.save(connection.c_Session);
                    continue;
                }

                continue;
            }

            // Parses the current command
            std::tie(currentCommand, currentCommandArgs) = ServerCommand::parse(&buffer[0]);

            // ----
            // Checks how the server should respond, and process the operation
            // ----

            switch (currentCommand)
            {
                // Client introduces
                case ServerCommand::SMTPServerCommand::HELLO: {
                    if (!Responses::handleHelo(&sock_fd, ssl, currentCommandArgs,
                            connPhasePt, sockaddrIn)) {
                        err = true;
                        goto end;
                    }

                    break;
                }

                // Client requests exit
                case ServerCommand::SMTPServerCommand::QUIT: {
                    Responses::handleQuit(&sock_fd, ssl);
                    goto end;
                }

                // Handles 'MAIL_FROM'
                case ServerCommand::SMTPServerCommand::MAIL_FROM: {
                    if (!Responses::handleMailFrom(&sock_fd, ssl, currentCommandArgs,
                            result, connPhasePt)) {
                        err = true;
                        goto end;
                    }

                    break;
                }

                // Handles 'RCPT TO'
                case ServerCommand::SMTPServerCommand::RCPT_TO: {
                    if (!Responses::handleRcptTo(&sock_fd, ssl, currentCommandArgs, result,
                            connPhasePt, connection.c_Session)) {
                        err = true;
                        goto end;
                    };
                    break;
                }

                // Handles 'START TLS'
                case ServerCommand::SMTPServerCommand::START_TLS: {
                    DEBUG_ONLY(print << "Client requested TLS connection" << Logger::ConsoleOptions::ENDL)

                    // Writes the message
                    const char *message = ServerCommand::gen(220, "Go ahead", nullptr, 0);
                    Responses::write(&sock_fd, ssl, message, strlen(message));
                    delete message;

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
                    SSL_set_fd(ssl, sock_fd);

                    // Starts the SSL connection
                    if (SSL_accept(ssl) <= 0)
                    {
                        PREP_ERROR("OpenSSL Error", "Could not accept mailer ..")
                        ERR_print_errors_fp(stderr);
                    }

                    // Resets the state
                    connPhasePt = ConnPhasePT::PHASE_PT_INITIAL;

                    // Sets using ssl to true
                    usingSSL = true;

                    DEBUG_ONLY(print << "TLS connection initialized" << Logger::ConsoleOptions::ENDL)

                    // Breaks
                    break;
                }

                // The data section
                case ServerCommand::SMTPServerCommand::DATA: {
                    // Checks if the command is allowed
                    if (connPhasePt >= ConnPhasePT::PHASE_PT_MAIL_TO)
                    {
                        // Sends the response
                        const char *message = ServerCommand::gen(354, "", nullptr, 0);
                        Responses::write(&sock_fd, ssl, message, strlen(message));
                        connPhasePt = ConnPhasePT::PHASE_PT_DATA;
                        delete message;

                        // Breaks
                        break;
                    }

                    // Sends the message that action is not allowed
                    Responses::preContextBadSequence(&sock_fd, ssl, "RCPT TO");

                    // Breaks
                    break;
                }

                // The help command

                // Command not found
                default: {
                    Responses::syntaxError(&sock_fd, ssl);
                    err = true;
                    goto end;
                }
            }
        }

        // ----
        // End section, will shut down the socket, and perform other stopping operations
        // ----

        end:
            // If required free memory
            if (ssl != nullptr)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
                SSL_CTX_free(sslCtx);
            }

            // Closes the socket
            shutdown(sock_fd, SHUT_RDWR);

            // Deletes the sockaddrIn
            delete sockaddrIn;

            // Makes thread available
            _usedThreads--;

            #ifdef DEBUG
            print.setLevel(Logger::Level::LOGGER_WARNING);
            print << "Server closed transmission channel, mailer disconnected." << Logger::ConsoleOptions::ENDL;
            #endif
    }
};