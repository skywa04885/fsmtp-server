/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "./esmtp-auth.src.hpp"

namespace Fannst::FSMTPServer::ESMTPModules
{
    namespace Auth
    {
        bool handleAuth(const int *soc, SSL *ssl, const char *args, CassSession *cassSession)
        {
            SMTPAuthorizationProtocol protocol = SMTPAuthorizationProtocol::INVALID;
            std::size_t tl;
            bool rc = true;
            int rc1;
            BYTE ti;

            char *tok;
            char *argsC;
            char *hash;
            char *user;
            char *password;

            // ----
            // Creates the logger
            // ----

            #ifdef DEBUG
            Logger::Console print(Logger::Level::LOGGER_DEBUG, "ESMTP Authorization");
            print << "Client requested Authorization, started processing ..." << Logger::ConsoleOptions::ENDL;
            #endif

            // ----
            // Checks if there are arguments at all
            // ----

            // Checks if the arguments contain something
            if (args == nullptr || args[0] == '\0')
            { // Arguments are empty
                char *msg = ServerCommand::gen(501, "Syntax Error: please supply authorization protocol and parameter .", nullptr, 0);
                Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                free(msg);
                // Sets error code and goes to the end
                rc = false;
                goto handleAuthEnd;
            }

            // ----
            // Parses the arguments
            // ----

            // Creates the argsC variable, with an copy of the args
            argsC = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(strlen(&args[0]), 0)));
            strcpy(&argsC[0], &args[0]);

            // Creates the tokenizer
            tok = strtok(&argsC[0], " ");

            // Starts looping
            ti = 0;
            while (tok != nullptr)
            {
                // Checks if the argument is useable
                if (ti == 0)
                { // Is the authorization mechanism

                    // Allocates required temp buffer
                    char *tMech = reinterpret_cast<char *>(malloc(16));

                    // Checks which mechanism we're using
                    if (tok[0] == 'P')
                    { // Starts with p
                        // Copies the data
                        memcpy(&tMech[0], &tok[0], 5);
                        tMech[5] = '\0';

                        // Checks if it matches
                        if (strcmp(&tMech[0], "PLAIN") == 0)
                        {
                            protocol = SMTPAuthorizationProtocol::PLAIN;
                        }
                    } else if (tok[0] == 'L')
                    {
                        // Copies the data
                        memcpy(&tMech[0], &tok[0], 5);
                        tMech[5] = '\0';

                        // Checks if it matches
                        if (strcmp(&tMech[0], "LOGIN") == 0)
                        {
                            protocol = SMTPAuthorizationProtocol::LOGIN;
                        }
                    } else if (tok[0] == 'D')
                    {
                        // Copies the data
                        memcpy(&tMech[0], &tok[0], 10);
                        tMech[10] = '\0';

                        // Checks if it matches
                        if (strcmp(&tMech[0], "DIGEST-MD5") == 0)
                        {
                            protocol = SMTPAuthorizationProtocol::DIGEST_MD5;
                        }
                    } else if (tok[0] == 'C')
                    {
                        // Copies the data
                        memcpy(&tMech[0], &tok[0], 8);
                        tMech[8] = '\0';

                        // Checks if it matches
                        if (strcmp(&tMech[0], "CRAM-MD5") == 0)
                        {
                            protocol = SMTPAuthorizationProtocol::CRAM_MD5;
                        }
                    } else if (tok[0] == 'O')
                    {
                        // Copies the data
                        memcpy(&tMech[0], &tok[0], 8);
                        tMech[8] = '\0';

                        // Checks if it matches
                        if (strcmp(&tMech[0], "OAUTH10A") == 0)
                        {
                            protocol = SMTPAuthorizationProtocol::OAUTH10A;
                        }
                    } else if (tok[0] == 'O')
                    {
                        // Copies the data
                        memcpy(&tMech[0], &tok[0], 11);
                        tMech[11] = '\0';

                        // Checks if it matches
                        if (strcmp(&tMech[0], "OAUTHBEARER") == 0)
                        {
                            protocol = SMTPAuthorizationProtocol::OAUTH_BEARER;
                        }
                    } else if (tok[0] == 'G')
                    {
                        // Copies the data
                        memcpy(&tMech[0], &tok[0], 6);
                        tMech[6] = '\0';

                        // Checks if it matches
                        if (strcmp(&tMech[0], "GSSAPI") == 0)
                        {
                            protocol = SMTPAuthorizationProtocol::GSS_API;
                        }
                    }

                    // Frees the allocated memory
                    free(tMech);
                } else if (ti == 1)
                { // Is the hash
                    // Allocates the memory, and stores copy
                    tl = strlen(&tok[0]);
                    hash = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(tl, 0)));
                    memcpy(&hash[0], &tok[0], tl);
                } else { ti++; break; };

                // Goes to the next token
                tok = strtok(nullptr, " ");
                ti++;
            }

            // ----
            // Free the memory
            // ----

            free(argsC);

            // ----
            // Evaluates the parsing
            // ----

            // Checks if the arguments are valid
            if (ti > 2)
            {
                // Sends the error
                char *msg = ServerCommand::gen(501,
                                               "Syntax Error: too many arguments for command 'AUTH'", nullptr,
                                               0);
                Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                delete(msg);
                // Sets error code and goes to the end
                rc = false;
                goto handleAuthEnd;
            } else if (ti == 1)
            {
                // Sends the error
                char *msg = ServerCommand::gen(501,
                                               "Syntax Error: too few arguments for command 'AUTH'", nullptr,
                                               0);
                Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                delete(msg);
                // Sets error code and goes to the end
                rc = false;
                goto handleAuthEnd;
            }

            // Checks if we implement the algorithm
            if (protocol == SMTPAuthorizationProtocol::INVALID || (protocol != SMTPAuthorizationProtocol::DIGEST_MD5
            && protocol != SMTPAuthorizationProtocol::PLAIN))
            {
                // Sends the error
                char *msg = ServerCommand::gen(504,
                                               "Parameter not implemented: authorization "
                                               "protocol not implemented", nullptr,0);
                Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                delete(msg);
                // Sets error code and goes to the end
                rc = false;
                goto handleAuthEnd;
            }

            // ----
            // Decodes the hash
            // ----

            switch (protocol)
            {
                // ----
                // Plain
                // ----

                case SMTPAuthorizationProtocol::PLAIN:
                {
                    // ----
                    // Decodes Base64 using OpenSSL
                    // ----

                    // Creates the origin size
                    tl = ALLOC_CAS_STRING(strlen(&hash[0]), 0);

                    // Creates the result rBuff
                    char *rBuff = (char *)malloc(tl);
                    memset(rBuff, '\0', tl);

                    // Initializes OpenSSL Bio for Base64
                    BIO *b64, *bmem;
                    b64 = BIO_new(BIO_f_base64());
                    bmem = BIO_new_mem_buf(hash, tl);
                    bmem = BIO_push(b64, bmem);

                    // Disables line breaks
                    BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);

                    // Reads the decoded value with its length
                    BIO_read(bmem, rBuff, tl);

                    // Frees the memory
                    BIO_free_all(bmem);

                    // ----
                    // Starts processing the string
                    // ----

                    std::size_t lastNullCharIndex = 0;
                    BYTE j = 0;

                    char *c = &rBuff[0];

                    // Loops over all the bytes
                    for (std::size_t i = 0; i < tl; i++)
                    {
                        // Checks if we have an null char
                        if (*c == '\0')
                        {
                            if (j == 0)
                            { // The first null byte
                                lastNullCharIndex = i;
                                j++;
                            } else if (j == 1)
                            { // The user

                                // Allocates memory for the user, and copies the value into it
                                user = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(
                                        strlen(&rBuff[lastNullCharIndex+1]), 0)));
                                strcpy(&user[0], &rBuff[lastNullCharIndex+1]);

                                // Increments the other stuff
                                lastNullCharIndex = i;
                                j++;
                            } else if (j == 2)
                            { // The password
                                // Allocates memory for the user, and copies the value into it
                                password = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(
                                        strlen(&rBuff[lastNullCharIndex+1]), 0)));
                                strcpy(&password[0], &rBuff[lastNullCharIndex+1]);

                                // Increments the other stuff
                                lastNullCharIndex = i;
                                j++;
                            }
                        }

                        // Goes to the next char
                        c++;
                    }
                }
            }

            // ----
            // Prints the data for debug
            // ----

            DEBUG_ONLY(print << "Authorization parsing complete, results: [user: " << user << ", password: "
                << password << "]" << Logger::ConsoleOptions::ENDL)

            // ----
            // Checks if the user is in the database and if the password is valid
            // ----

            {
                char *username = nullptr;
                char *domain = nullptr;

                // Splits the email
                if (Parsers::splitAddress(&user[0], &username, &domain) < 0)
                {
                    // Sends the error
                    char *msg = ServerCommand::gen(530,
                            "Authorization Error: Invalid email address",
                            nullptr,0);
                    Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                    delete(msg);
                    // Sets error code and goes to the end
                    rc = false;
                    goto handleAuthEnd;
                }

                // Creates a new user quick access
                Models::UserQuickAccess userQuickAccess{};

                // Performs the query
                rc1 = Models::UserQuickAccess::selectByDomainAndUsername(cassSession, domain, username, userQuickAccess);

                // ----
                // Frees the memory
                // ----

                free(username);
                free(domain);

                // ----
                // Evaluates the query
                // ----

                // Checks if the query went wrong
                if (rc1 == -1)
                { // Cassandra error
                    // Sends the error
                    char *msg = ServerCommand::gen(530,
                            "Authorization Error: Database did not respond",
                            nullptr,0);
                    Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                    delete(msg);
                    // Sets error code and goes to the end
                    rc = false;
                    goto handleAuthEnd;
                } else if (rc1 == -2)
                { // User not found
                    // Sends the error
                    char *msg = ServerCommand::gen(530,
                            "Authorization Error: User not found",
                            nullptr,0);
                    Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                    delete(msg);
                    // Sets error code and goes to the end
                    rc = false;
                    goto handleAuthEnd;
                }

                // ----
                // Checks if the password is valid
                // ----

                // Hashes the user supplied password
                char *hRet = nullptr;
                OpenSSL::sha256base64(&password[0], &hRet);

                // Compares it against the password in the database
                if (strcmp(&hRet[0], &userQuickAccess.u_Password[0]) != 0)
                {
                    // Sends the error
                    char *msg = ServerCommand::gen(530,
                            "Authorization Error: Invalid password",
                            nullptr,0);
                    Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                    delete(msg);
                    // Sets error code and goes to the end
                    rc = false;
                    goto handleAuthEnd;
                }

                // ----
                // Sends the success message
                // ----

                // Sends the message
                char *msg = ServerCommand::gen(235,
                        "Authentication successful",nullptr,0);
                Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                delete(msg);
            }

            // ----
            // The end
            // ----

        handleAuthEnd:

            // ----
            // Frees the memory
            // ----

            free(hash);

            // ----
            // Returns
            // ----

            return rc;
        }
    }
}