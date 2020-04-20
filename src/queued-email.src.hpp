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
        QueuedEmail(const long &m_Timestamp, const CassUuid &m_UUID, const long &m_Bucket, const CassUuid &m_UserUUID);

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

        long m_Timestamp;
        long m_Bucket;
        CassUuid m_UserUUID;
        CassUuid m_UUID;
    };
}