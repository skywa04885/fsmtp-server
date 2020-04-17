/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <iostream>

#include <cassandra.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../../pre.hpp"
#include "../../email.src.hpp"
#include "../server.src.hpp"

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
                Server::ConnPhasePT &phasePt, Models::UserQuickAccess *pUserQuickAccess);

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
                Server::ConnPhasePT &phasePt, struct sockaddr_in *sAddr);

        /**
         * Handles the quit command
         * @param soc
         * @param ssl
         */
        void handleQuit(const int *soc, SSL *ssl);

        /**
         * Handles the help command
         * @param soc
         * @param ssl
         */
        void handleHelp(const int *soc, SSL *ssl);

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
                Server::ConnPhasePT &phasePt, CassSession *cassSession, Models::UserQuickAccess *pUserQuickAccess);
    };
}