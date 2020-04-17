/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <cassandra.h>
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../server.src.hpp"
#include "../responses.src.hpp"
#include "../../user.src.hpp"

namespace Fannst::FSMTPServer::ESMTPModules
{
    typedef enum {
        INVALID = 0,
        PLAIN,
        LOGIN,
        DIGEST_MD5,
        CRAM_MD5,
        OAUTH10A,
        OAUTH_BEARER,
        GSS_API,
        MD5
    } SMTPAuthorizationProtocol;

    namespace Auth
    {
        bool handleAuth(const int *soc, SSL *ssl, const char *args, CassSession *cassSession);
    }
}