/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <fannst_libcompose.hpp>

#include "server.src.hpp"
#include "./responses.src.hpp"
#include "modules/esmtp.src.hpp"
#include "modules/esmtp-auth.src.hpp"

namespace Fannst::FSMTPServer::Server
{
    std::atomic<int> _usedThreads(0);
    int _maxThreads = MAX_THREADS;

    /**
         * Loads the passphrase from file
         * @param buffer
         * @param size
         * @param rwflag
         * @param u
         * @return
         */
    int sslConfigureContextLoadPassword(char *buffer, int size, int rwflag, void *u)
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
        SSL_CTX_set_default_passwd_cb(ctx, &sslConfigureContextLoadPassword);

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
        Logger::Console print(Logger::Level::LOGGER_DEBUG, printPrefix.c_str());
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
        Models::UserQuickAccess *userQuickAccess = nullptr;

        // ----
        // The connection status variables
        // ----

        bool err = false;           // If the current thread has errored out

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

            if (ssl == nullptr) readLen = recv(sock_fd, &buffer[0], 1024, 0);
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
                    // Sets the status
                    connPhasePt = ConnPhasePT::PHASE_PT_DATA_END;

                    // Sends the message
                    char *msg = ServerCommand::gen(250, "Ok: queued as 0", nullptr, 0);
                    Responses::write(&sock_fd, ssl, msg, strlen(msg));
                    free(msg);

                    // ----
                    // Parses the message
                    // ----

                    {
                        Timer t("MimeParser");

                        char *headers = nullptr;
                        char *body = nullptr;

                        // ----
                        // Separates the headers from the body
                        // ----

                        Fannst::FSMTPServer::MIMEParser::separateHeadersAndBody(dataBuffer.c_str(), &headers, &body);

                        // ----
                        // Parses the headers
                        // ----

                        // Parses
                        if (Fannst::FSMTPServer::MIMEParser::parseHeaders(headers, result.m_FullHeaders) < 0)
                        {
                            std::cout << "Went wrong ..." << std::endl;
                        }

                        // Checks for the required data, such as content type etc
                        char *klw = reinterpret_cast<char *>(malloc(65));
                        char *cmpT = reinterpret_cast<char *>(malloc(65));

                        std::size_t hValueLen;

                        for (auto &h : result.m_FullHeaders)
                        {
                            hValueLen = strlen(&h.h_Value[0]);

                            // ----
                            // Turns the key into an lower case char
                            // ----

                            // Copies the string value
                            strcpy(&klw[0], &h.h_Key[0]);

                            // Turns it into lower case
                            for (char *p = &klw[0]; *p != '\0'; p++)
                            {
                                *p = static_cast<char>(tolower(*p));
                            }

                            // ----
                            // Checks if the header is useful to us
                            // ----

                            if (klw[0] == 'm')
                            { // Starts with m, possible "message-id"

                                // Copies the text
                                cmpT[10] = '\0';
                                memcpy(&cmpT[0], &klw[0], 10);

                                // Compares the strings
                                if (strcmp(&cmpT[0], "message-id") == 0)
                                { // Is the message id

                                    // Allocates the memory for the message id
                                    result.m_MessageID = reinterpret_cast<const char *>(malloc(
                                            ALLOC_CAS_STRING(hValueLen, 0)));

                                    // Copies the value
                                    memcpy(&const_cast<char *>(result.m_MessageID)[0], &h.h_Value[0], hValueLen + 1);
                                }
                            } else if (klw[0] == 's')
                            { // starts with 's' possible "subject"

                                // Copies the text
                                cmpT[7] = '\0';
                                memcpy(&cmpT[0], &klw[0], 7);

                                // Compares the strings
                                if (strcmp(&cmpT[0], "subject") == 0)
                                {
                                    // Allocates the memory for the message id
                                    result.m_Subject = reinterpret_cast<const char *>(malloc(
                                            ALLOC_CAS_STRING(hValueLen, 0)));

                                    // Copies the value
                                    memcpy(&const_cast<char *>(result.m_Subject)[0], &h.h_Value[0], hValueLen + 1);
                                }
                            } else if (klw[0] == 'c')
                            { // starts with 'c', possible "content-type"
                                // Copies the text
                                cmpT[12] = '\0';
                                memcpy(&cmpT[0], &klw[0], 12);

                                // Compares the strings
                                if (strcmp(&cmpT[0], "content-type") == 0)
                                {
                                    std::vector<const char *> nkValues{};
                                    std::map<const char *, const char *> kValues{};

                                    // ----
                                    // Starts parsing
                                    // ----

                                    // Parses the arguments
                                    MIMEParser::parseHeaderParameters(h.h_Value, nkValues, kValues);

                                    // Gets the content type
                                    result.m_ContentType = MIMEParser::getContentType(nkValues.at(0));

                                    // Finds the boundary
                                    for (auto it = kValues.begin(); it != kValues.end(); it++)
                                    {
                                        // If not starting with b, it is not boundary
                                        if (it->first[0] != 'b') continue;

                                        // Checks if it is the boundary
                                        if (strcmp(&it->first[0], "boundary") == 0)
                                        {
                                            // Allocates the memory
                                            result.m_Boundary = reinterpret_cast<char *>(
                                                    malloc(ALLOC_CAS_STRING(strlen(&it->second[0]), 0)));

                                            // Copies the boundary
                                            memcpy(const_cast<char *>(&result.m_Boundary[0]),
                                                    &it->second[0], strlen(&it->second[0]) + 1);
                                        }
                                    }

                                    // ----
                                    // Frees the memory
                                    // ----

                                    // Frees the no key values
                                    for (const char *a : nkValues) free(const_cast<char *>(a));

                                    // Frees the keyed values
                                    for (auto it = kValues.begin(); it != kValues.end(); it++)
                                    {
                                        free(const_cast<char *>(it->first));
                                        free(const_cast<char *>(it->second));
                                    }
                                }
                            } else if (klw[0] == 'f')
                            { // Starts with 'f', possible "from"

                                // Copies the text
                                cmpT[4] = '\0';
                                memcpy(&cmpT[0], &klw[0], 4);

                                // Compares the strings
                                if (strcmp(&cmpT[0], "from") == 0)
                                {
                                    // Parses the addresses
                                    if (MIMEParser::parseAddressList(&h.h_Value[0], result.m_From) < 0)
                                    {
                                        // TODO: Handle error
                                    }
                                }
                            } else if (klw[0] == 't')
                            { // Starts with 't', possible 'to'

                                // Copies the text
                                cmpT[2] = '\0';
                                memcpy(&cmpT[0], &klw[0], 2);

                                // Compares the strings
                                if (strcmp(&cmpT[0], "to") == 0)
                                {
                                    // Parses the addresses
                                    if (MIMEParser::parseAddressList(&h.h_Value[0], result.m_To) < 0)
                                    {
                                        // TODO: Handle error
                                    }
                                }
                            }
                        }

                        // ----
                        // Parses the message body with the required algorithm
                        // ----

                        switch (result.m_ContentType)
                        {
                            // Parsing "multipart/alternative" using boundary
                            case Types::MimeContentType::MULTIPART_ALTERNATIVE:
                            {
                                MIMEParser::parseMultipartAlternativeBody(&body[0], result.m_Boundary, result.m_Content);
                            }
                        }

                        // ----
                        // Frees the memory
                        // ----

                        free(klw);
                        free(cmpT);
                        free(headers);
                        free(body);
                    }

                    // ----
                    // Configures final properties
                    // ----

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

                    // Saves the email
                    std::cout << result << std::endl;
//                    result.save(connection.c_Session);

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
                case ServerCommand::SMTPServerCommand::HELLO: {
                    if (!ESMTPModules::Default::handleHello(&sock_fd, ssl, currentCommandArgs, connPhasePt, sockaddrIn))
                    {
                        err = true;
                        goto end;
                    }
                    break;
                }
                case ServerCommand::SMTPServerCommand::MAIL_FROM: {
                    if (!ESMTPModules::Default::handleMailFrom(&sock_fd, ssl, currentCommandArgs, result, connPhasePt, userQuickAccess))
                    {
                        err = true;
                        goto end;
                    }
                    break;
                }
                case ServerCommand::SMTPServerCommand::RCPT_TO: {
                    if (!ESMTPModules::Default::handleRcptTo(&sock_fd, ssl, currentCommandArgs, result, connPhasePt, connection.c_Session, userQuickAccess))
                    {
                        err = true;
                        goto end;
                    }
                    break;
                }
                case ServerCommand::SMTPServerCommand::START_TLS: {
                    // Writes the message
                    char *message = ServerCommand::gen(220, "Ok: continue", nullptr, 0);
                    Responses::write(&sock_fd, ssl, message, strlen(message));
                    free(message);

                    // ----
                    // Initializes OpenSSL
                    // ----

                    DEBUG_ONLY(print << "Initializing TLS connection ..." << Logger::ConsoleOptions::ENDL)

                    // Creates the context
                    const SSL_METHOD *method = SSLv23_server_method();
                    sslCtx = SSL_CTX_new(method);
                    if (!sslCtx)
                    {
                        PREP_ERROR("SSL Error", "Could not create context ..")
                        ERR_print_errors_fp(stderr);

                        err = true;
                        goto end;
                    }

                    // Assigns the keys and certificate
                    if (sslConfigureContext(sslCtx) < 0)
                    {
                        PREP_ERROR("SSL Error", "Could not configure context ..")
                        ERR_print_errors_fp(stderr);

                        err = true;
                        goto end;
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
                    connPhasePt = Server::ConnPhasePT::PHASE_PT_INITIAL;

                    DEBUG_ONLY(print << "Initialized TLS connection ..." << Logger::ConsoleOptions::ENDL)

                    break;
                }

                // ----
                // DATA START
                // ----

                case ServerCommand::SMTPServerCommand::DATA: {
                    // Checks if the command is allowed
                    if (connPhasePt >= ConnPhasePT::PHASE_PT_MAIL_TO)
                    {
                        // Sends the response
                        char *message = ServerCommand::gen(354, "End data with <CR><LF>.<CR><LF>", nullptr, 0);
                        Responses::write(&sock_fd, ssl, message, strlen(message));
                        connPhasePt = ConnPhasePT::PHASE_PT_DATA;
                        free(message);
                        break;
                    }

                    // Sends the message that action is not allowed
                    Responses::preContextBadSequence(&sock_fd, ssl, "RCPT TO");

                    // Breaks
                    break;
                }
                case ServerCommand::SMTPServerCommand::QUIT: {
                    ESMTPModules::Default::handleQuit(&sock_fd, ssl);
                    goto end;
                }
                case ServerCommand::SMTPServerCommand::HELP: {
                    ESMTPModules::Default::handleHelp(&sock_fd, ssl);
                    break;
                }
                case ServerCommand::SMTPServerCommand::AUTH: {
                    if (!ESMTPModules::Auth::handleAuth(&sock_fd, ssl, currentCommandArgs, connection.c_Session, &userQuickAccess))
                    {
                        err = true;
                        goto end;
                    }
                    break;
                }
                default: {
                    Responses::syntaxError(&sock_fd, ssl);
                    err = true;
                    goto end;
                }
            }

            // ----
            // Frees the memory
            // ----

            free(const_cast<char *>(currentCommandArgs));
        }

        // ----
        // End section, will shut down the socket, and perform other stopping operations
        // ----

        end:
            // If required free memory
            if (ssl != nullptr) { SSL_shutdown(ssl); SSL_free(ssl); }
            if (sslCtx != nullptr) SSL_CTX_free(sslCtx);

            if (userQuickAccess != nullptr) free(userQuickAccess);

            // Closes the socket
            shutdown(sock_fd, SHUT_RDWR);

            // Deletes the sockaddrIn
            free(sockaddrIn);

            // Makes thread available
            _usedThreads--;

            #ifdef DEBUG
            print.setLevel(Logger::Level::LOGGER_WARNING);
            print << "Server closed transmission channel, mailer disconnected." << Logger::ConsoleOptions::ENDL;
            #endif
    }
};