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
                // Generates the message
                const char * message= serverCommand::gen(250, "OK Proceed");
                // Transmits the message
                write(soc, message, strlen(message));
            }

            /**
             * Sends that an bad sequence occurred
             * @param ssl
             * @param reqMsg
             */
            void preContextBadSequence(int *soc, const char *reqMsg)
            {
                // Generates the message
                const char *message = serverCommand::gen(503, reqMsg);
                // Transmits the message
                write(soc, message, strlen(message));
            }

            /**
             * Sends an syntax error
             * @param ssl
             */
            void syntaxError(int *soc)
            {
                // Generates the message
                const std::string message = serverCommand::generate(501, "");
                // Transmits the message
                write(soc, message.c_str(), message.length());
            }

            // ----
            // The request handlers
            // ----

            bool handleMailFrom(int *soc, const std::string &args, models::Email &email, ConnPhasePT &phase)
            {
                // Checks if the sequence is correct
                if (phase < ConnPhasePT::PHASE_PT_MAIL_TO)
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

                // Checks if the sender is a person from Fannst, and possibly tries to relay message
                // TODO: Relay check

                // Sends continue
                preContextProceed(soc);
                // Returns, everything went fine
                return true;
            }

            bool handleHelo(int *soc, const std::string &args, ConnPhasePT &phase, ConnectionThreadParams &params)
            {
                // Generates the message, I'm using the C string, because it just is faster.
                char temp[80];
                strcat(temp, inet_ntoa(params.client->sin_addr));
                strcat(temp, " nice to meet you !\r\n");

                // Generates the message itself
                const char *message = serverCommand::gen(250, temp);

                // Sends the message
                write(soc, message, strlen(message));

                // Updates the phase
                phase = ConnPhasePT::PHASE_PT_HELLO;

                // Returns true
                return true;
            }

            bool handleRcptTo(int *soc, const std::string &args, models::Email &email, ConnPhasePT &phase,
                    CassSession *cassSession)
            {
                // Checks if the sequence is correct
            }
        };
    };
};