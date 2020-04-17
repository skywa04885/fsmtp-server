/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>

#include <memory.h>

#include "../pre.hpp"
#include "../flib/strings.hpp"

namespace Fannst::FSMTPServer::ServerCommand
{
    typedef enum {
        INVALID,
        HELLO,
        MAIL_FROM,
        RCPT_TO,
        DATA,
        START_TLS,
        QUIT,
        HELP,
        AUTH
    } SMTPServerCommand;

    /**
     * Parses an mailer to server command
     * @param buf
     * @return
     */
    std::tuple<SMTPServerCommand, const char *> parse(char *buf);

    /**
     * Generates an response with code
     * @param code
     * @param param
     * @param listParams
     * @param listParamsN
     * @return
     */
    char *gen(int code, const char *param, const char *listParams[], char listParamsN);

    /**
     * Converts command enum to string
     * @param command
     * @return
     */
    const char *serverCommandToString(const SMTPServerCommand& command);
};