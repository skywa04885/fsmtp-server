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

namespace serverCommand
{
    typedef enum {
        INVALID,
        HELLO,
        MAIL_FROM,
        RCPT_TO,
        DATA,
        START_TLS,
        QUIT
    } SMTPServerCommand;

    std::tuple<SMTPServerCommand, std::string> parse(const std::string& buffer);

    std::string generate(int code, const char *param);

    const char *serverCommandToString(const SMTPServerCommand& command);
};