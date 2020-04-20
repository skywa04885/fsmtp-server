/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <vector>
#include <iostream>

#include <cassandra.h>

#include "pre.hpp"

namespace Fannst::FSMTPServer::Models
{
    class QueuedEmail
    {
    public:
        /**
         * Default constructor for an queued email
         * @param m_timestamp
         * @param m_uuid
         */
        QueuedEmail(const long &m_timestamp, const CassUuid &m_uuid);

        /**
         * Saves an queued email
         * @param session
         * @return
         */
        BYTE save(CassSession *session);

        /**
         * Gets message to sent using the queue
         * @param cassSession
         * @param result
         * @param count
         * @return
         */
        static BYTE get(CassSession *cassSession, std::vector<QueuedEmail> &result, const int &count);
    private:
        long m_timestamp;
        CassUuid m_uuid;
    };
}