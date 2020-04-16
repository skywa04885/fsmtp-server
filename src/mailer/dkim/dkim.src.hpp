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
#include <fannst_libencoding.hpp>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#include "../../logger.src.hpp"
#include "../../pre.hpp"

#define FANNST_DKIM_TOTAL_PARTS 11
#define FANNST_DKIM_MAX_SIG_LL 66

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
        const char *d_PKey{nullptr};        // Path of private key

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
    int formatSignature(char *parts[FANNST_DKIM_TOTAL_PARTS], char **ret, bool nl);

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
    int sign(const char *raw, char **sigRet, const DKIMHeaderConfig *config);

    /**
     * Splits MIME message into two sections
     * @param raw
     * @param headerRet
     * @param bodyRet
     * @return
     */
    int parseMimeMessage(const char *raw, char **headerRet, char **bodyRet);

    /**
     * Canonicalizes headers using the relaxed method
     * @param raw
     * @param ret
     */
    void canonicalizeHeadersRelaxed(const char *raw, char **ret);

    /**
     * Canonicalizes body using the relaxed method
     * @param raw
     * @param ret
     */
    void canonicalizeBodyRelaxed(const char *raw, char **ret);

    char *cleanWhitespace(const char *a, std::size_t aLen);

    namespace OpenSSL
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
        int rsaSha256genSig(const char *raw, const char *pKeyFile, char **hRet);
    }

    /**
     * Gets the index of an char
     * @param a
     * @param b
     * @return
     */
    inline int findChar(char *a, char b)
    {
        // Creates the result
        int res = 0;

        // Infinite loop
        for (;;)
        {
            // Checks if we should break
            if (*a == '\0') return -1;
            else if (b == *a) return res;

            // Goes to the next char in memory
            a++;
            res++;
        }
    }
}