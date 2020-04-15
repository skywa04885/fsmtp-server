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

#include "../../logger.src.hpp"
#include "../../pre.hpp"

#define FANNST_DKIM_TOTAL_PARTS 11

namespace Fannst::FSMTPServer::DKIM
{
    typedef enum {
        DCA_RELAXED_RELAXED = 0,
        DCA_RELAXED_SIMPLE,
        DCA_SIMPLE_RELAXED,
        DCA_SIMPLE_SIMPLE
    } DKIMCanAlgorithms;

    typedef struct {
        long d_SignDate{};                  // Sign time in MS since last epoch
        long d_ExpireDate{};                // Expire time in MS since last epoch
        const char *d_Domain{nullptr};      // The domain name
        const char *d_KeyS{nullptr};        // The domain key selector

        // The default set parameters
        DKIMCanAlgorithms d_Ago{
            DKIMCanAlgorithms::DCA_RELAXED_RELAXED
        };                                  // The DKIM algorithms being used for canonicalization
    } DKIMHeaderConfig;

    /**
     * Formats an array of signature parts, into the final signature
     * @param parts
     * @param ret
     * @return
     */
    int formatSignature(const char *parts[FANNST_DKIM_TOTAL_PARTS], char *ret);

    /**
     * Builds an signature part
     * @param key
     * @param kv
     * @return
     */
    char *buildSigPart(const char *key, const char *kv);

    /**
     * Signs an MIME message
     * @param raw
     * @param config
     * @return
     */
    int sign(const char *raw, char *sigRet, const DKIMHeaderConfig *config);

    /**
     * Splits MIME message into two sections
     * @param raw
     * @param headerRet
     * @param bodyRet
     * @return
     */
    int parseMimeMessage(const char *raw, char *headerRet, char *bodyRet);
}