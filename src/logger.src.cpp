/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "logger.src.hpp"

namespace logger {
    Console::Console(const Level &c_Level, const char *c_Prefix) {
        this->c_Level = c_Level;
        this->c_Prefix = c_Prefix;
    }

    Console::~Console() {
        return;
    }

    void Console::setLevel(const Level &c_Level) {
        this->c_Level = c_Level;
    }
};