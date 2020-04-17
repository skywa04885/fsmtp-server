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
                Server::ConnPhasePT &phasePt)
        {
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

            // Parses the email address from the args, if invalid: send syntax error
            if (Parsers::parseAddress(args, email.m_TransportFrom) < 0)
            {
                // Writes the syntax error
                Responses::syntaxError(soc, ssl);
                // Returns false, close connection
                return false;
            }

            // Checks if the sender is a person from Fannst, and possibly tries to relay message
            // TODO: Relay check

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
                char *message = ServerCommand::gen(501, "Empty EHLO/HELO command not allowed, closing connection.", nullptr, 0);
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
                const char *argList[] = {"STARTTLS", "HELP", "AUTH DIGEST-MD5"};
                // Generates the message
                message = ServerCommand::gen(250, &temp[0], argList, sizeof(argList) / sizeof(const char *));
            } else
            {
                // Creates the arguments list
                const char *argList[] {"HELP", "AUTH DIGEST-MD5 PLAIN LOGIN"};
                // Generates the message
                message = ServerCommand::gen(250, &temp[0], argList, sizeof(argList) / sizeof(const char *));
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
            char *message = ServerCommand::gen(214, "Please visit https://software.fannst.nl for basic help, the Github repository is https://github.com/skywa04885/fsmtp-server", nullptr, 0);
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
                Server::ConnPhasePT &phasePt, CassSession *cassSession)
        {
            // ----
            // Checks the sequence, if it is correct
            // ----

            if (phasePt < Server::ConnPhasePT::PHASE_PT_MAIL_FROM)
            {
                // Writes the error
                Responses::preContextBadSequence(soc, ssl, "MAIL FROM");
                // Returns false
                return true;
            }

            // ----
            // Checks if the arguments are valid, empty is false
            // ----

            if (args == nullptr || strlen(args) == 0)
            {
                Responses::syntaxError(soc, ssl);
                return false;
            }

            // Parses the address
            if (Parsers::parseAddress(args, email.m_TransportTo) < 0)
            {
                Responses::syntaxError(soc, ssl);
                return false;
            }

            // ----
            // Splits the domain name from the address
            // ----

            // Splits the address
            char *emailCstr = const_cast<char *>(email.m_TransportTo.e_Address.c_str());
            char *username = nullptr;
            char *domain = nullptr;
            char *tok = strtok(emailCstr, "@");

            // Checks if the string is too long, and if we need to stop
            if (strlen(emailCstr) > 256)
            {
                // Sends the error message
                char *message = ServerCommand::gen(471,
                                                   "Address too large, would cause stack overflow ..", nullptr, 0);
                Responses::write(soc, ssl, message, strlen(message));
                free(message);

                PREP_ERROR("Refused parsing", "Address longer then 256 chars, preventing high memory usage ..")
                return false;
            }

            // Loop over the occurrences, char because of the small size
            unsigned char i = 0;
            while (tok != nullptr)
            {
                if (i == 0)
                {
                    // Allocates space on the stack
                    username = reinterpret_cast<char *>(alloca(strlen(tok)));
                    // Copies the string, including the  '\0'
                    memcpy(username, tok, strlen(tok) + 1);
                } else if (i == 1)
                {
                    // Allocates space on the stack
                    domain = reinterpret_cast<char *>(alloca(strlen(tok)));
                    // Copies the string, including the  '\0'
                    memcpy(domain, tok, strlen(tok) + 1);
                } else break;

                // Finds the next token
                tok = strtok(nullptr, "@");
                i++; // Increments i
            }

            // Checks if the parsing went good, if not return server error
            if (username == nullptr || domain == nullptr)
            {
                // Sends the response message
                char *message = ServerCommand::gen(471, "could not parse address ..",
                                                   nullptr, 0);
                Responses::write(soc, ssl, message, strlen(message));
                free(message);

                // Returns that there was an error
                return false;
            }

            // ----
            // Performs user check from Apache Cassandra
            // ----

            // Finds the receiver on our server
            Models::UserQuickAccess userQuickAccess;
            int rc = Models::UserQuickAccess::selectByDomainAndUsername(cassSession, domain, username,
                                                                        userQuickAccess);

            // Checks if the user was found
            if (rc == -1)
            {
                // Sends the response message
                char *message = ServerCommand::gen(471, "Could not perform cassandra query ..",
                                                   nullptr, 0);
                Responses::write(soc, ssl, message, strlen(message));
                free(message);

                // Returns false
                return false;
            } else if (rc == -2)
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

            // Sets the phase
            phasePt = Server::ConnPhasePT::PHASE_PT_MAIL_TO;

            return true;
        }
    }
}