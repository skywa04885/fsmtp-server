/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <chrono>
#include <thread>

#include "../logger.src.hpp"
#include "../pre.hpp"
#include "../db/cassandra.src.hpp"
#include "../queued-email.src.hpp"
#include "../mailer/mailer.src.hpp"

namespace Fannst::FSMTPServer::Sender
{
    /**
     * Runs the email sender queue as thread
     * @param argc
     * @param argv
     */
    void run(int *argc, char ***argv);
}