#pragma once

#include <tuple>
#include <cstring>
#include <iostream>

namespace Fannst
{
    typedef enum {
        CM_HELO,
        CM_MAIL_FROM,
        CM_MAIL_TO,
        CM_DATA,
        CM_QUIT,
        CM_START_TLS
    } ClientCommand;

    /**
     * Parses SMTP Server response
     * @param raw
     * @return
     */
    int parseCommand(const char *raw, int& code, char *arguments);

    const char *gen(const ClientCommand &clientCommand, const bool& argsInline, const char *args);
}
