/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <iostream>
#include <cstring>
#include <malloc.h>

#include "../email.src.hpp"
#include "../types/mime.src.hpp"
#include "../pre.hpp"
#include "../parsers/mime-parser.src.hpp"
#include "../debug/timer.src.hpp"

namespace Fannst::FSMTPServer::Server
{
    /**
     * Parses an raw mime message, and stores it into an email
     * @param raw
     * @param target
     * @return
     */
    BYTE parseMessage(const char *raw, Models::Email &target);
}