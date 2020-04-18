/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <functional>
#include <iostream>

#include "../../mongoose/mongoose.h"

namespace Fannst::FSMTPServer::DS_API
{
    /**
     * Runs the DS Api
     * @param port
     * @param argc
     * @param argv
     */
    void run(int port, int *argc, char ***argv);
}