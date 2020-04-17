/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <cstring>
#include <malloc.h>
#include <vector>

#include "../pre.hpp"
#include "../email.src.hpp"

namespace Fannst::FSMTPServer::MIMEParser
{
    /**
     * Separates the headers from the body in MIME message
     * @param raw
     * @param headRet
     * @param bodyRet
     * @return
     */
    BYTE separateHeadersAndBody(const char *raw, char **headRet, char **bodyRet);

    /**
     * Parses the headers into an vector
     * @param raw
     * @param headers
     * @return
     */
    BYTE parseHeaders(const char *raw, std::vector<Fannst::FSMTPServer::Models::EmailHeader> &headers);

    /**
     * Splits an email address up in the domain, and username
     * @param raw
     * @param username
     * @param domain
     * @return
     */
    int splitAddress(const char *raw, char **username, char**domain);
}