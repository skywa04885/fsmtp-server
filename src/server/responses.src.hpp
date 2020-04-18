/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <openssl/ssl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./commands.src.hpp"
#include "../pre.hpp"
#include "../email.src.hpp"
#include "server.src.hpp"
#include "../user.src.hpp"

namespace Fannst::FSMTPServer::Responses
{
    // ----
    // The global methods, such as write
    // ----

    /**
     * Writes an message to an encrypted socket or plain text socket
     * @param soc
     * @param ssl
     * @param msg
     * @param msg_len
     */
    inline void write(const int *soc, SSL *ssl, const char *msg, int msg_len)
    {
        if (ssl == nullptr)
        {
            if (send(*soc, msg, msg_len, 0) < 0) {
                PREP_ERROR("Could not send message", msg)
            }
        } else
        {
            // Sends the message over the socket
            SSL_write(ssl, msg, msg_len);

            // TODO: Check for errors
        }
    }

    // ----
    // The pre-programmed responses
    // ----

    /**
     * Sends that the mailer may continue
     * @param ssl
     */
    inline void preContextProceed(const int *soc, SSL *ssl)
    {
        char *message = ServerCommand::gen(250, "Ok: proceed", nullptr, 0);
        write(soc, ssl, message, strlen(message));
        free(message);
    }

    /**
     * Sends that an bad sequence occurred
     * @param ssl
     * @param reqMsg
     */
    inline void preContextBadSequence(const int *soc, SSL *ssl, const char *reqMsg)
    {
        char *message = ServerCommand::gen(503, reqMsg, nullptr, 0);
        write(soc, ssl, message, strlen(message));
        free(message);
    }

    /**
     * Sends an syntax error
     * @param ssl
     */
    inline void syntaxError(const int *soc, SSL *ssl)
    {
        char *message = ServerCommand::gen(501, "Syntax error", nullptr, 0);
        write(soc, ssl, message, strlen(message));
        free(message);
    }
}