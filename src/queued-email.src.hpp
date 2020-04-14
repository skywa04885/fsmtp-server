/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma  once

#include <cassandra.h>

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
        int save(CassSession *session);
    private:
        long m_timestamp;
        CassUuid  m_uuid;
    };
}