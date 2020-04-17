/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "./esmtp.src.hpp"
#include "../responses.src.hpp"

namespace Fannst::FSMTPServer::ESMTPModules
{
    namespace Default
    {
        /**
         * Handles the Mail From command
         * @param soc
         * @param ssl
         * @param args
         * @param email
         * @param phasePt
         * @return
         */
        bool handleMailFrom(const int *soc, SSL *ssl, const char *args, Models::Email &email,
                Server::ConnPhasePT &phasePt, Models::UserQuickAccess *pUserQuickAccess)
        {
            // ----
            // Verifies the data and sequence
            // ----

            // Checks if the sequence is correct
            if (phasePt < Server::ConnPhasePT::PHASE_PT_HELLO)
            {
                // Writes the error
                Responses::preContextBadSequence(soc, ssl, "HELO");
                // Returns true, so mailer can retry
                return true;
            }

            // Checks if the args are empty, if so return syntax error
            if (args == nullptr || strlen(args) == 0)
            {
                // Writes the syntax error
                Responses::syntaxError(soc, ssl);
                // Returns false, close connection
                return false;
            }

            // ----
            // Parses the address
            // ----

            // Parses the email address from the args, if invalid: send syntax error
            if (Parsers::parseAddress(args, email.m_TransportFrom) < 0)
            {
                // Writes the syntax error
                Responses::syntaxError(soc, ssl);
                // Returns false, close connection
                return false;
            }

            // ----
            // Checks if the sender is from fannst.nl, if so make sure he is authorized to do this
            // ----

            {
                char *username = nullptr;
                char *domain = nullptr;

                // Parses the address
                Parsers::splitAddress(&email.m_TransportFrom.e_Address[0], &username, &domain);

                // Checks if it is an fannst sender
                if (strcmp(&domain[0], GE_DOMAIN) == 0 && pUserQuickAccess == nullptr)
                {
                    // Sends the error
                    char *msg = ServerCommand::gen(503,
                            "Error: Authorization is required for own-domain "
                            "senders",nullptr,0);
                    Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                    delete(msg);
                }

                // Frees the left memory
                free(username);
                free(domain);
            }

            // Sends continue
            Responses::preContextProceed(soc, ssl);

            // Sets the phase
            phasePt = Server::ConnPhasePT::PHASE_PT_MAIL_FROM;

            // Returns, everything went fine
            return true;
        }

        /**
         * Handles the Hello command
         * @param soc
         * @param ssl
         * @param args
         * @param phasePt
         * @param sAddr
         * @return
         */
        bool handleHello(const int *soc, SSL *ssl, const char *args,
                Server::ConnPhasePT &phasePt, struct sockaddr_in *sAddr)
        {
            // ----
            // Checks if args are there
            // ----

            if (args == nullptr || args[0] == '\0')
            {
                // Sends the response
                char *message = ServerCommand::gen(501,
                        "Empty EHLO/HELO command not allowed, closing connection", nullptr,
                        0);
                Responses::write(soc, ssl, message, strlen(message));
                free(message);

                // Closes the connection
                return false;
            }

            // ----
            // Generates the message title
            // ----

            char *temp = reinterpret_cast<char *>(malloc(64));
            temp[0] = '\0';
            strcat(&temp[0], "smtp.fannst.nl at your service, [");
            strcat(&temp[0], inet_ntoa(sAddr->sin_addr));
            strcat(&temp[0], "]");

            // ----
            // Sends the message
            // ----

            char *message;

            if (ssl == nullptr)
            {
                // Creates the arguments list
                const char *argList[] = {"STARTTLS", "HELP", "AUTH PLAIN"};
                // Generates the message
                message = ServerCommand::gen(250, &temp[0], argList,
                        sizeof(argList) / sizeof(const char *));
            } else
            {
                // Creates the arguments list
                const char *argList[] {"HELP", "AUTH PLAIN"};
                // Generates the message
                message = ServerCommand::gen(250, &temp[0], argList,
                        sizeof(argList) / sizeof(const char *));
            }
            Responses::write(soc, ssl, message, strlen(message));
            free(message);

            // Updates the phase
            phasePt = Server::ConnPhasePT::PHASE_PT_HELLO;

            // Returns true
            return true;
        }

        /**
         * Handles the quit command
         * @param soc
         * @param ssl
         */
        void handleQuit(const int *soc, SSL *ssl)
        {
            char *message = ServerCommand::gen(221, "Goodbye !", nullptr, 0);
            Responses::write(soc, ssl, message, strlen(message));
            free(message);
        }

        /**
         * Handles the help command
         * @param soc
         * @param ssl
         */
        void handleHelp(const int *soc, SSL *ssl)
        {
            char *message = ServerCommand::gen(214,
                    "Please visit https://software.fannst.nl for basic help, the Github repository "
                    "is https://github.com/skywa04885/fsmtp-server, Author: Luke A.C.A. Rieff", nullptr, 0);
            Responses::write(soc, ssl, message, strlen(message));
            free(message);
        }

        /**
         * Handles the mail to command
         * @param soc
         * @param ssl
         * @param args
         * @param email
         * @param phasePt
         * @param cassSession
         * @return
         */
        bool handleRcptTo(const int *soc, SSL *ssl, const char *args, Models::Email &email,
                Server::ConnPhasePT &phasePt, CassSession *cassSession, Models::UserQuickAccess *pUserQuickAccess)
        {
            bool rc = true;

            // ----
            // Checks the sequence, if it is correct
            // ----

            if (phasePt < Server::ConnPhasePT::PHASE_PT_MAIL_FROM)
            {
                // Writes the error
                Responses::preContextBadSequence(soc, ssl, "MAIL FROM");
                // No memory allocated, so just return
                return false;
            }

            // ----
            // Checks if the arguments are valid, empty is false
            // ----

            if (args == nullptr || strlen(args) == 0)
            {
                Responses::syntaxError(soc, ssl);
                // No memory allocated, so just return
                return false;
            }

            // Parses the address
            if (Parsers::parseAddress(args, email.m_TransportTo) < 0)
            {
                Responses::syntaxError(soc, ssl);
                // No memory allocated, so just return
                return false;
            }

            // ----
            // Splits the domain name from the address
            // ----

            // Splits the address
            char *username = nullptr;
            char *domain = nullptr;

            // Checks if the parsing went good, if not return server error
            if (Parsers::splitAddress(&email.m_TransportTo.e_Address.c_str()[0], &username, &domain) < 0)
            {
                // Sends the response message
                char *message = ServerCommand::gen(501, "Syntax Error: could not parse address",
                                                   nullptr, 0);
                Responses::write(soc, ssl, message, strlen(message));
                free(message);

                // Sets the return code and goes to the end
                rc = false;
                goto handleRcptToEnd;
            }

            // ----
            // Checks if the email is being forwarded
            // ----

            if (strcmp(domain, GE_DOMAIN) != 0)
            { // Message needs to be forwarded

                // ----
                // Checks if relaying is allowed
                // ----

                if (!pUserQuickAccess)
                {
                    // Writes the error
                    char *msg = ServerCommand::gen(503,
                            "Error: Authorization is required for relaying",
                            nullptr,0);
                    Responses::write(soc, ssl, &msg[0], strlen(&msg[0]));
                    delete(msg);

                    // Sets the return code and goes to the end
                    rc = false;
                    goto handleRcptToEnd;
                }

                // ----
                // Sets the relaying to true
                // ----

                // TODO: Fix relaying
            } else
            {
                // ----
                // Performs user check from Apache Cassandra
                // ----

                // Finds the receiver on our server
                Models::UserQuickAccess userQuickAccess;
                int rc2 = Models::UserQuickAccess::selectByDomainAndUsername(cassSession, domain, username,
                                                                             userQuickAccess);

                // Checks if the user was found
                if (rc2 == -1)
                {
                    // Sends the response message
                    char *message = ServerCommand::gen(471, "Could not perform cassandra query",
                                                       nullptr, 0);
                    Responses::write(soc, ssl, message, strlen(message));
                    free(message);

                    // Returns false
                    return false;
                } else if (rc2 == -2)
                {
                    // Sends the response message
                    char *message = ServerCommand::gen(551, "User not local", nullptr, 0);
                    Responses::write(soc, ssl, message, strlen(message));
                    free(message);

                    // Returns false
                    return false;
                }

                // Sets the user id to the email user id
                email.m_UserUUID = userQuickAccess.u_Uuid;

                // Sends continue
                Responses::preContextProceed(soc, ssl);
            }

            // Sets the phase
            phasePt = Server::ConnPhasePT::PHASE_PT_MAIL_TO;

            // ----
            // The end
            // ----
        handleRcptToEnd:

            // ----
            // Frees memory
            // ----

            free(username);
            free(domain);

            return rc;
        }
    }
}