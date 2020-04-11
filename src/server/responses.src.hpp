/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <openssl/ssl.h>
#include "./commands.src.hpp"


namespace server
{
    namespace responses
    {
        /**
         * The methods, which will choose which way it needs to response
         */

        /**
         * TLS Responses
         */

        namespace tls
        {
            // When the client may continue
            void preContextProceed(int *socket, SSL *ssl)
            {

            }

            // When another command is first of all required
            void preContextOtherRequired(int *socket, SSL *ssl, const char *reqMsg)
            {

            }

            // When an syntax error has been made
            void syntaxError(int *socket, SSL *ssl)
            {
                const char *message = serverCommand::generate(501, "");
            }
        };

        /**
         * Plain text responses
         */

        namespace plain
        {
            void send(int *socket, const char *msg, std::size_t msg_len)
            {
//                RSP_SV_DEBUG_ONLY(std::cout << __FILE__ << ": " << __LINE__);
            }
        };
    };
};