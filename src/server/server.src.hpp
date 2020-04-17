/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <tuple>
#include <thread>
#include <fstream>
#include <memory.h>
#include <atomic>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../db/cassandra.src.hpp"
#include "../logger.src.hpp"
#include "commands.src.hpp"
#include "../email.src.hpp"
#include "../parsers/mail-parser.src.hpp"
#include "../user.src.hpp"
#include "../pre.hpp"

#define MAX_THREADS 6

namespace Fannst::FSMTPServer::Server
{
    typedef struct {
        int *clientSocket;
        struct sockaddr_in *client;
    } ConnectionThreadParams;

    typedef enum {
        PHASE_PT_INITIAL,
        PHASE_PT_HELLO,
        PHASE_PT_MAIL_FROM,
        PHASE_PT_MAIL_TO,
        PHASE_PT_DATA,
        PHASE_PT_DATA_END,
        PHASE_PT_QUIT
    } ConnPhasePT;


    int run(const unsigned int& port, int *argc, char ***argv);

    void connectionThread(struct sockaddr_in *sockaddrIn, int sock_fd);

    void sendMessage(const int *socket, std::string& message, Logger::Console& print);

    /**
 * Loads the passphrase from file
 * @param buffer
 * @param size
 * @param rwflag
 * @param u
 * @return
 */
    int sslConfigureContextLoadPassword(char *buffer, int size, int rwflag, void *u);

    /**
     * Configures an OpenSSL context for OpenSSL Sockets
     * @param ctx
     * @return
     */
    int sslConfigureContext(SSL_CTX *ctx);
};
