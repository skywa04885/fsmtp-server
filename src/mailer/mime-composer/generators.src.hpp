/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <iostream>
#include <cstring>
#include <random>
#include <string>
#include <malloc.h>

#include "../../pre.hpp"

namespace Fannst::FSMTPServer::Mailer::Composer
{
    /**
     * Generates an message id
     * @param mesRet
     * @param mesRetLen
     */
    void _generateMessageID(char **mesRet, std::size_t *mesRetLen);

    /**
     * Generates an message boundary
     * @param bouRet
     * @param bouRetLen
     */
    void _generateBoundary(char **bouRet, std::size_t *bouRetLen);
}