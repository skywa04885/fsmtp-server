/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <openssl/ssl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./commands.src.hpp"
#include "../pre.hpp"
#include "../email.src.hpp"
#include "../parsers/mail-parser.src.hpp"
#include "server.src.hpp"
#include "../user.src.hpp"

namespace server
{
    namespace responses
    {
        /**
         * Namespace for TLS request handlers
         */
        namespace tls
        {
            // ----
            // The global methods, such as write
            // ----

            /**
             * Writes an response to SSL socket
             * @param ssl
             * @param msg
             * @param msg_len
             */
            void write(SSL *ssl, const char *msg, int msg_len)
            {
                // Sends the message over the socket
                SSL_write(ssl, msg, msg_len);

                // TODO: Check for errors
            }

            // ----
            // The pre-programmed responses
            // ----

            /**
             * Sends that the client may continue
             * @param ssl
             */
            void preContextProceed(SSL *ssl)
            {
                // Generates the message
                const std::string message = serverCommand::generate(250, "OK Proceed");
                // Transmits the message
                write(ssl, message.c_str(), message.length());
            }

            /**
             * Sends that an bad sequence occurred
             * @param ssl
             * @param reqMsg
             */
            void preContextBadSequence(SSL *ssl, const char *reqMsg)
            {
                // Generates the message
                const std::string message = serverCommand::generate(503, reqMsg);
                // Transmits the message
                write(ssl, message.c_str(), message.length());
            }

            /**
             * Sends an syntax error
             * @param ssl
             */
            void syntaxError(SSL *ssl)
            {
                // Generates the message
                const std::string message = serverCommand::generate(501, "");
                // Transmits the message
                write(ssl, message.c_str(), message.length());
            }

            /**
             * Handles 'MAIL FROM' command
             * @param ssl
             * @param command
             * @param args
             * @param email
             * @param phase
             * @return
             */
            bool handleMailFrom(SSL *ssl, const std::string &args, models::Email &email, const ConnPhaseEC &phase)
            {
                // Checks if the sequence is correct
                if (phase <= ConnPhaseEC::PHASE_EC_HELLO)
                {
                    // Writes the error
                    preContextBadSequence(ssl, "HELO");
                    // Returns true, so client can retry
                    return true;
                }

                // Checks if the args are empty, if so return syntax error
                if (args.empty())
                {
                    // Writes the syntax error
                    syntaxError(ssl);
                    // Returns false, close connection
                    return false;
                }

                // Parses the email address from the args, if invalid: send syntax error
                if (parsers::parseAddress(args, email.m_TransportFrom) < 0)
                {
                    // Writes the syntax error
                    syntaxError(ssl);
                    // Returns false, close connection
                    return false;
                }

                // Checks if the sender is a person from Fannst, and possibly tries to relay message
                // TODO: Relay check

                // Sends continue
                preContextProceed(ssl);
                // Returns, everything went fine
                return true;
            }

            bool handleHelo(SSL *ssl, const std::string &args, ConnPhasePT &phase, ConnectionThreadParams &params)
            {
                // Generates the message, I'm using the C string, because it just is faster.
                char temp[80];
                temp[0] = '\0';
                strcat(temp, inet_ntoa(params.client->sin_addr));
                strcat(temp, " nice to meet you !");

                // Sends the response message
                const char *message = serverCommand::gen(250, temp);
                write(ssl, message, strlen(message));
                delete message;

                // Updates the phase
                phase = ConnPhasePT::PHASE_PT_HELLO;

                // Returns true
                return true;
            }
        };

        /**
         * Namespace for PLAIN request handlers
         */
        namespace plain
        {
            // ----
            // The global methods, such as write
            // ----

            void write(const int *soc, const char *msg, int msg_len)
            {
                // Sends the message, and checks for possible errors
                if (send(*soc, msg, msg_len, 0) < 0) {
                    PREP_ERROR("Could not send message", msg)
                }
            }

            // ----
            // The pre-programmed responses
            // ----

            /**
             * Sends that the client may continue
             * @param ssl
             */
            void preContextProceed(int *soc)
            {
                const char *message = serverCommand::gen(250, "OK Proceed");
                write(soc, message, strlen(message));
                delete message;
            }

            /**
             * Sends that an bad sequence occurred
             * @param ssl
             * @param reqMsg
             */
            void preContextBadSequence(int *soc, const char *reqMsg)
            {
                const char *message = serverCommand::gen(503, reqMsg);
                write(soc, message, strlen(message));
                delete message;
            }

            /**
             * Sends an syntax error
             * @param ssl
             */
            void syntaxError(int *soc)
            {
                const char *message = serverCommand::gen(501, "");
                write(soc, message, strlen(message));
                delete message;
            }

            // ----
            // The request handlers
            // ----

            bool handleMailFrom(int *soc, const std::string &args, models::Email &email, ConnPhasePT &phase)
            {
                // Checks if the sequence is correct
                if (phase < ConnPhasePT::PHASE_PT_HELLO)
                {
                    // Writes the error
                    preContextBadSequence(soc, "HELO");
                    // Returns true, so client can retry
                    return true;
                }

                // Checks if the args are empty, if so return syntax error
                if (args.empty())
                {
                    // Writes the syntax error
                    syntaxError(soc);
                    // Returns false, close connection
                    return false;
                }

                // Parses the email address from the args, if invalid: send syntax error
                if (parsers::parseAddress(args, email.m_TransportFrom) < 0)
                {
                    // Writes the syntax error
                    syntaxError(soc);
                    // Returns false, close connection
                    return false;
                }

                // Prints to the console
                std::cout << "Email receiving from: " << args << std::endl;

                // Checks if the sender is a person from Fannst, and possibly tries to relay message
                // TODO: Relay check

                // Sends continue
                preContextProceed(soc);

                // Sets the phase
                phase = ConnPhasePT::PHASE_PT_MAIL_FROM;

                // Returns, everything went fine
                return true;
            }

            bool handleHelo(int *soc, const std::string &args, ConnPhasePT &phase, ConnectionThreadParams &params)
            {
                // Generates the message, I'm using the C string, because it just is faster.
                char temp[80];
                temp[0] = '\0';
                strcat(temp, inet_ntoa(params.client->sin_addr));
                strcat(temp, " nice to meet you !");

                // Sends the response message
                const char *message = serverCommand::gen(250, temp);
                write(soc, message, strlen(message));
                delete message;

                // Updates the phase
                phase = ConnPhasePT::PHASE_PT_HELLO;

                // Returns true
                return true;
            }

            void handleQuit(int *soc)
            {
                // Sends the response message
                const char *message = serverCommand::gen(221, "");
                write(soc, message, strlen(message));
                delete message;
            }

            bool handleRcptTo(int *soc, const std::string &args, models::Email &email, ConnPhasePT &phase,
                    CassSession *cassSession)
            {
                // Checks if the sequence is correct
                if (phase < ConnPhasePT::PHASE_PT_MAIL_FROM)
                {
                    // Writes the error
                    preContextBadSequence(soc, "MAIL FROM");
                    // Returns false
                    return false;
                }

                // Checks if the arguments are empty
                if (args.empty())
                {
                    // Writes an syntax error
                    syntaxError(soc);
                    // Returns false
                    return false;
                }

                // Parses the address
                if (parsers::parseAddress(args, email.m_TransportTo) < 0)
                {
                    // Writes the syntax error
                    syntaxError(soc);
                    // Returns false, close connection
                    return false;
                }

                // Splits the address
                char *emailCstr = const_cast<char *>(email.m_TransportTo.e_Address.c_str());
                char *username = nullptr;
                char *domain = nullptr;
                char *tok = strtok(emailCstr, "@");

                // Checks if the string is too long, and if we need to stop
                if (strlen(emailCstr) > 256)
                {
                    PREP_ERROR("Refused parsing", "Address longer then 256 chars, preventing stack overflow ..")

                    // Sends the error message
                    const char *message = serverCommand::gen(471, "Address too large, would cause stack overflow ..");
                    write(soc, message, strlen(message));
                    delete message;

                    // Returns false
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
                    const char *message = serverCommand::gen(471, "could not parse address ..");
                    write(soc, message, strlen(message));
                    delete message;

                    // Returns that there was an error
                    return false;
                }

                // ----
                // Performs user check from Apache Cassandra
                // ----

                // Finds the receiver on our server
                models::UserQuickAccess userQuickAccess;
                int rc = models::UserQuickAccess::selectByDomainAndUsername(cassSession, domain, username,
                        userQuickAccess);

                // Checks if the user was found
                if (rc == -1)
                {
                    // Sends the response message
                    const char *message = serverCommand::gen(471, "Could not perform cassandra query ..");
                    write(soc, message, strlen(message));
                    delete message;

                    // Returns false
                    return false;
                } else if (rc == -2)
                {
                    // Sends the response message
                    const char *message = serverCommand::gen(551, "");
                    write(soc, message, strlen(message));
                    delete message;

                    // Returns false
                    return false;
                }

                // Sets the user id to the email user id
                email.m_UserUUID = userQuickAccess.u_Uuid;

                // Sends continue
                preContextProceed(soc);

                // Sets the phase
                phase = ConnPhasePT::PHASE_PT_MAIL_TO;

                // Returns, everything went fine
                return true;
            }
        };
    };
};