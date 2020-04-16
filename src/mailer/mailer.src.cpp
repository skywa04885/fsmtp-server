#include "mailer.src.hpp"

namespace Fannst
{

    Mailer::Mailer(Fannst::Composer::Options &c_ComposerOptions):
        c_ComposerOptions(c_ComposerOptions)
    {}

    /**
     * Sends an email
     * @return
     */
    int Mailer::sendMessage(const char *extIp) {

        // ----
        // Initializes OpenSSL
        // ----

        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        // ----
        // Global sending variables
        // ----

        std::string messageBody;
        std::vector<Fannst::dns::ResolverMXRecord> mxRecords;
        char *tok, *domain = nullptr, *addressCStr;
        int rc;
        char i, j;

        // ----
        // Prepares the message body
        // ----

        messageBody = Fannst::Composer::compose(this->c_ComposerOptions);

        // ----
        // Starts the sendMessage loop
        // ----

        for (Fannst::Types::EmailAddress &address : this->c_ComposerOptions.o_To)
        {
            // ----
            // Creates the logger if debug
            // ----

            DEBUG_ONLY(FSMTPServer::Logger::Console print(FSMTPServer::Logger::Level::LOGGER_DEBUG, "Mailer"))

            // ----
            // Resolves the records
            // ----

            // Splits the email address
            addressCStr = reinterpret_cast<char *>(alloca(sizeof(char) * address.e_Address.length()));
            strcpy(&addressCStr[0], &address.e_Address.c_str()[0]);
            tok = strtok(addressCStr, "@");
            i = 0;
            for (;;)
            {
                if (i == 0);
                else if (i == 1)
                {
                    // Allocates the memory
                    domain = reinterpret_cast<char *>(alloca(sizeof(char) * strlen(tok) + 1));
                    // Copies the memory
                    memcpy(domain, tok, strlen(tok) + 1);
                } else break;
                tok = strtok(nullptr, "@");
                // Increments i
                i++;
            }

            // Checks if it is there
            if (domain == nullptr)
            {
                // TODO: Handle error log
                continue;
            }

            // Resolves the records
            mxRecords.clear();
            rc = Fannst::dns::resolveMX(domain, mxRecords);

            #ifdef DEBUG
            print << "Resolved records for " << domain << ": " << FSMTPServer::Logger::ConsoleOptions::ENDL;
            for (auto &record : mxRecords)
            {
                std::cout << record << std::endl;
            }
            #endif

            // Checks if there went something wrong
            if (rc < 0)
            {
                // TODO: Handle error log
                continue;
            }

            // ----
            // Gets the remote address, and initializes data
            // ----

            // Grabs random mx record
            Fannst::dns::ResolverMXRecord &record = mxRecords.at(0);

            // Resolves the ip address
            char ipAddress[45];
            rc = Fannst::dns::resolveIpAddress(record.r_Exchange, &ipAddress[0]);
            DEBUG_ONLY(print << "Resolved ip address of: " << record.r_Exchange << ", to: " <<
            ipAddress << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL)

            // Checks if the resolve went successfully
            if (rc < 0)
            {
                // TODO: Handle error log
                continue;
            }

            j = 0;
            for (;;)
            {
                if (j == 0)
                {
                    // Transmits the message, using StartTLS
                    rc = transmitMessage(ipAddress, this->c_ComposerOptions.o_From.at(0), address,
                            messageBody, true, extIp);

                    // Checks if the message was sent successfully
                    if (rc == 0) break;

                    // If not, perform action accordingly
                    if (rc == -3)
                    {
                        j++;
                    } else break;
                } else if (j >= 1)
                {
                    rc = transmitMessage(ipAddress, this->c_ComposerOptions.o_From.at(0), address,
                            messageBody, false, extIp);

                    // Checks if the message was sent successfully
                    if (rc == 0) break;

                    break;
                }
            }
        }

        return 0;
    }

    int transmitMessage(char *ipAddress, Fannst::Types::EmailAddress &mailFrom,
            Fannst::Types::EmailAddress &mailTo, std::string &messageBody, bool usingSSL, const char *extIp)
    {
        // Gets the time
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        long a = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        // Performs dkim
        Fannst::FSMTPServer::DKIM::DKIMHeaderConfig config{};
        config.d_Domain = "fannst.nl";
        config.d_ExpireDate = a + 600;
        config.d_SignDate = a;
        config.d_KeyS = "default";
        config.d_PKey = "../keys/dkim/private-key.pem";

        char *sigRet = nullptr;
        Fannst::FSMTPServer::DKIM::sign(messageBody.c_str(), &sigRet, &config);


        std::cout << std::endl << std::endl << std::endl << "----------" << std::endl;

        std::cout << &sigRet[0] << std::endl;

        std::cout << "----------" << std::endl;
//        std::exit(-1);

        // ----
        // Creates the logger if enabled
        // ----

        DEBUG_ONLY(Fannst::FSMTPServer::Logger::Console print(Fannst::FSMTPServer::Logger::Level::LOGGER_DEBUG, "Message Transmission"))

        struct sockaddr_in server{};
        int rc, sock_fd;

        // Creates the server sock struct
        server.sin_addr.s_addr = inet_addr(ipAddress);
        server.sin_port = htons(25);
        server.sin_family = AF_INET;

        // ----
        // Initializes the socket
        // ----

        // Creates the socket
        sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock_fd < 0)
        {
            // TODO: Handle error log
            PREP_ERROR("Socket Error", "Could not create socket")
            return -1;
        }

        // Creates the timeout
        struct timeval timeout{};
        timeout.tv_sec = 7;
        timeout.tv_usec = 0;

        DEBUG_ONLY(print << "Setting socket options" << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL)

        // Sets the timeout
        if (
                setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO,
                           reinterpret_cast<char *>(&timeout), sizeof(timeout)) < 0 ||
                setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO,
                           reinterpret_cast<char *>(&timeout), sizeof(timeout)) < 0
                )
        {
            PREP_ERROR("Socket Error", "Could not set socket timeout")
            return -1;
        }

        // ----
        // Connects the socket
        // ----

        DEBUG_ONLY(print << "Connecting to: " << ipAddress << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL)

        if (connect(sock_fd, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0)
        {
            PREP_ERROR("Could not connect to server", ipAddress)
            return -2;
        }

        // ----
        // Loop for handling data
        // ----

        char buffer[512];
        char arguments[256];
        int code;
        rc = 0;
        MailerState state = MailerState::MST_INITIAL;

        SSL *ssl = nullptr;
        SSL_CTX *sslCtx = nullptr;

        for (;;)
        {
            // ----
            // Receives the message, and parses the command
            // ----

            // Clears the buffers
            memset(&buffer[0], 0, sizeof(buffer));
            memset(&arguments[0], 0, sizeof(arguments));

            // Receives data, if socket closed: break
            if (Fannst::FSMTPClient::SocketHandler::read(&sock_fd, ssl, buffer, 512) < 0)
            {
                break;
            }

            // Parses the commands
            Fannst::parseCommand(&buffer[0], code, &arguments[0]);

            // ----
            // Checks what the server has said
            // ----

            switch (code)
            {
                case 220:
                { // Initial greeting
                    if (state == MailerState::MST_INITIAL) state = MailerState::MST_HELO;
                    else if (state == MailerState::MST_START_TLS) state = MailerState::MST_START_TLS_CONNECT;

                    // Breaks
                    break;
                }
                case 250:
                {
                    // Checks what the next command is supposed to be
                    if (state == MailerState::MST_HELO) state = MailerState::MST_MAIL_FROM;
                    else if (state == MailerState::MST_MAIL_FROM) state = MailerState::MST_MAIL_TO;
                    else if (state == MailerState::MST_MAIL_TO) state = MailerState::MST_DATA;
                    else if (state == MailerState::MST_DATA_START) state = MailerState::MST_DATA_END;
                    else if (state == MailerState::MST_HELO_START_TLS_START) state = MailerState::MST_START_TLS;

                    // Breaks
                    break;
                }
                case 354:
                {
                    if (state == MailerState::MST_DATA) state = MailerState::MST_DATA_START;

                    // Breaks
                    break;
                }
                case 221:
                { // Exit

                    // Closes socket
                    goto end;
                }
                case 501:
                {
                    rc = -3;
                    goto end;
                }
                case 550:
                {
                    rc = -1;
                    goto end;
                }
            }

            // ----
            // Checks how to response
            // ----

            switch (state)
            {
                case MailerState::MST_START_TLS:
                {
                    if (!Fannst::FSMTPClient::SocketHandler::handleStartTlsCommand(&sock_fd, ssl))
                        goto end;

                    break;
                }

                case MailerState::MST_START_TLS_CONNECT:
                {
                    // ----
                    // Initializes the ssl connection or something
                    // ----

                    DEBUG_ONLY(print << "Initializing TLS" << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL)

                    // The SSL method
                    const SSL_METHOD *sslMethod = SSLv23_client_method();

                    // Creates the SSL context
                    sslCtx = SSL_CTX_new(sslMethod);
                    if (!sslCtx)
                    {
                        PREP_ERROR("OpenSSL Error", "Could not create context")
                        ERR_print_errors_fp(stderr);

                        // Closes the socket
                        rc = -1;
                        goto end;

                    }

                    // ----
                    // Starts using ssl
                    // ----

                    ssl = SSL_new(sslCtx);
                    if (!ssl)
                    {
                        PREP_ERROR("OpenSSL Error", "Could not create TLS")
                        ERR_print_errors_fp(stderr);

                        // Closes the socket
                        rc = -1;
                        goto end;
                    }

                    // Sets the socket
                    SSL_set_fd(ssl, sock_fd);

                    // Connects the SSL socket
                    if (SSL_connect(ssl) <= 0)
                    {
                        PREP_ERROR("OpenSSL Error", "Could not connect TLS")
                        ERR_print_errors_fp(stderr);

                        // Closes the socket
                        rc = -1;
                        goto end;
                    }

                    DEBUG_ONLY(print << "TLS Initialized, sending EHLO again" <<
                    ipAddress << Fannst::FSMTPServer::Logger::ConsoleOptions::ENDL)

                    // ----
                    // Sends the required hello message
                    // ----

                    // Sets the state to hello
                    state = MailerState::MST_HELO;

                    // Sends the message
                    if (!Fannst::FSMTPClient::SocketHandler::handleHelo(&sock_fd, ssl, extIp))
                        goto end;

                    break;
                }

                case MailerState::MST_HELO:
                {
                    if (!Fannst::FSMTPClient::SocketHandler::handleHelo(&sock_fd, ssl, extIp))
                        goto end;

                    if (usingSSL) {
                        state = MailerState::MST_HELO_START_TLS_START;
                    }

                    break;
                }

                case MailerState::MST_MAIL_FROM:
                {
                    if (!Fannst::FSMTPClient::SocketHandler::handleMailFrom(&sock_fd, ssl, mailFrom))
                        goto end;

                    break;
                }

                case MailerState::MST_MAIL_TO:
                {
                    if (!Fannst::FSMTPClient::SocketHandler::handleMailTo(&sock_fd, ssl, mailTo))
                        goto end;

                    break;
                }

                case MailerState::MST_DATA:
                {
                    if (!Fannst::FSMTPClient::SocketHandler::handleDataCommand(&sock_fd, ssl))
                        goto end;

                    break;
                }

                case MailerState::MST_DATA_START:
                {
                    if (!Fannst::FSMTPClient::SocketHandler::handleDataTransmission(&sock_fd, ssl, sigRet, strlen(&sigRet[0])))
                        goto end;

                    break;
                }

                case MailerState::MST_DATA_END:
                {
                    if (!Fannst::FSMTPClient::SocketHandler::handleQuit(&sock_fd, ssl))
                        goto end;

                    break;
                }
            }
        }

        // ----
        // The end, closes the socket and frees memory
        // ----

    end:
        shutdown(sock_fd, SHUT_RDWR);

        // Clears signature
        free(sigRet);

        // Clears the SSL stuff
        if (ssl != nullptr)
        {
            SSL_free(ssl);
            SSL_CTX_free(sslCtx);
        }

        // Returns that it was sent successfully
        return rc;
    }
};