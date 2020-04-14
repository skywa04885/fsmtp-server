/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "queued-email.src.hpp"

namespace Fannst::FSMTPServer::Models
{
    /**
     * Default constructor for an queued email
     * @param m_timestamp
     * @param m_uuid
     */
    QueuedEmail::QueuedEmail(const long &m_timestamp, const CassUuid &m_uuid):
        m_timestamp(m_timestamp), m_uuid(m_uuid) {}

    /**
    * Saves an queued email
    * @param session
    * @return
    */
    int QueuedEmail::save(CassSession *session) {
        return 0;
    }
}