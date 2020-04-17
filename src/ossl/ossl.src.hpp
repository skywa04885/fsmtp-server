/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <cstring>
#include <memory.h>
#include <iostream>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include "../pre.hpp"

namespace Fannst::FSMTPServer::OpenSSL
{
    /**
     * Performs SHA256 hash, and returns the base 64 version
     * @param raw
     * @param hRet
     */
    void sha256base64(const char *raw, char **hRet);

    /**
     * Performs RSA-SHA256 Signature, and returns the base 64 version
     * @param raw
     * @param hRet
     */
    int rsaSha256genSig(const char *raw,
                        const char *pKeyFile, char **hRet);
}