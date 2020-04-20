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

#include <cassandra.h>

#include "../pre.hpp"
#include "../types/mime.src.hpp"

namespace Fannst::FSMTPServer::Cassandra::Helpers
{
    /**
     * Gets the email address from an column
     * @param value
     * @return
     */
    BYTE getEmailAddressFromColumn(const CassValue *value, Types::EmailAddress &target);

    /**
     * Gets the header from an column
     * @param value
     * @return
     */
    BYTE getHeadersFromColumn(const CassValue *value, Types::MimeHeader &target);
}