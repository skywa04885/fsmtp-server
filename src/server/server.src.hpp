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
#include <string>
#include <tuple>
#include <thread>
#include <fstream>
#include <memory.h>
#include <atomic>
#include <openssl/ssl.h>

#include "../db/cassandra.src.hpp"
#include "../logger.src.hpp"
#include "commands.src.hpp"
#include "../email.src.hpp"
#include "../parsers/mail-parser.src.hpp"
#include "../user.src.hpp"
#include "../pre.hpp"
#include "../gui-gtk/main-window.src.hpp"

#define MAX_THREADS 6

namespace server
{
    typedef struct {
        int *clientSocket;
        struct sockaddr_in *client;
    } ConnectionThreadParams;

    typedef enum {
        PHASE_PT_INITIAL = 0,
        PHASE_PT_HELLO,
        PHASE_PT_MAIL_FROM,
        PHASE_PT_MAIL_TO,
        PHASE_PT_DATA,
        PHASE_PT_DATA_END,
        PHASE_PT_QUIT
    } ConnPhasePT;

    typedef enum {
        PHASE_EC_INITIAL = 0,
        PHASE_EC_HELLO,
        PHASE_eC_START_TLS,
        PHASE_EC_MAIL_FROM,
        PHASE_EC_MAIL_TO,
        PHASE_EC_DATA,
        PHASE_EC_DATA_END,
        PHASE_EC_QUIT
    } ConnPhaseEC;

    #ifdef GUI
    typedef struct {
        std::string s_RemoteAddr;
        const int s_SocNum;
        bool s_StillOpen;
        bool s_SSL;
        bool s_Success;
        long s_TimeStamp;
    } ServerMailThread;
    #endif

    int run(const unsigned int& port);

    void connectionThread(ConnectionThreadParams params);

    void sendMessage(const int *socket, std::string& message, logger::Console& print);
};