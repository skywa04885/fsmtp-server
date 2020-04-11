/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <cassandra.h>

#include "../logger.src.hpp"

namespace cassandra
{
    class Connection
    {
    public:
        Connection(const char *c_Hosts, bool& success);
        ~Connection();

        CassSession *c_Session;
    private:
        CassFuture *c_ConnectFuture;
        CassCluster *c_Cluster;
        const char *c_Hosts;
    };
};