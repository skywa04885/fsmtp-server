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
#include "../../ossl/ossl.src.hpp"

#define FANNST_DKIM_TOTAL_PARTS 11
#define FANNST_DKIM_MAX_SIG_LL 66

namespace Fannst::FSMTPServer::DKIM
{
    // Enumerator for canonicalization type
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
    int formatSignature(char *parts[FANNST_DKIM_TOTAL_PARTS],
            char **ret, bool nl);

    /**
     * Builds an signature part
     * @param key
     * @param kv
     * @return
     */
    void buildSigPart(const char *key, const char *kv, char **ret);

    /**
     * Signs an MIME message
     * @param raw
     * @param config
     * @return
     */
    int sign(const char *raw, char **sigRet,
            const DKIMHeaderConfig *config);

    /**
     * Splits MIME message into two sections
     * @param raw
     * @param headerRet
     * @param bodyRet
     * @return
     */
    int parseMimeMessage(const char *raw, char **headerRet,
            char **bodyRet);

    /**
     * Canonicalizes headers using the relaxed method
     * @param raw
     * @param ret
     */
    void canonicalizeHeadersRelaxed(const char *raw, char **ret);

    /**
     * Generates the headers field
     * @param raw
     * @param ret
     */
    void generateHeadersField(const char *raw, char **ret);

    /**
     * Canonicalizes body using the relaxed method
     * @param raw
     * @param ret
     */
    void canonicalizeBodyRelaxed(const char *raw, char **ret);

    void cleanWhitespace(const char *a, std::size_t aLen, char **ret);

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

    /**
 * Checks if the header should be used
 * @param h
 * @return
 */
    inline bool shouldUseHeader(const char *h)
    {
        char *cTmp = reinterpret_cast<char *>(alloca(11));

        if (h[0] == 'f')
        {
            // Copies the memory
            memcpy(&cTmp[0], &h[0], 4);
            cTmp[4] = '\0';

            // Checks if it matches
            if (strcmp(&cTmp[0], "from") == 0) return true;
        } else if (h[0] == 't')
        {
            // Copies the memory
            memcpy(&cTmp[0], &h[0], 2);
            cTmp[2] = '\0';

            // Checks if it matches
            if (strcmp(&cTmp[0], "to") == 0) return true;
        } else if (h[0] == 's')
        {
            // Copies the memory
            memcpy(&cTmp[0], &h[0], 7);
            cTmp[7] = '\0';

            // Checks if it matches
            if (strcmp(&cTmp[0], "subject") == 0) return true;
        } else if (h[0] == 'd')
        {
            // Copies the memory
            memcpy(&cTmp[0], &h[0], 4);
            cTmp[4] = '\0';

            // Checks if it matches
            if (strcmp(&cTmp[0], "date") == 0) return true;

            // Copies the string
            memcpy(&cTmp[0], &h[0], 14);
            cTmp[14] = '\0';

            // Checks if it matches
            if (strcmp(&cTmp[0], "dkim-signature") == 0) return true;
        } else if (h[0] == 'k')
        {
            // Copies the memory
            memcpy(&cTmp[0], &h[0], 8);
            cTmp[8] = '\0';

            // Checks if it matches
            if (strcmp(&cTmp[0], "keywords") == 0) return true;
        } else if (h[0] == 'm')
        {
            // Copies the memory
            memcpy(&cTmp[0], &h[0], 10);
            cTmp[10] = '\0';

            // Checks if it matches
            if (strcmp(&cTmp[0], "message-id") == 0) return true;

            // Copies the string
            memcpy(&cTmp[0], &h[0], 12);
            cTmp[12] = '\0';

            // Checks if it matches
            if (strcmp(&cTmp[0], "mime-version") == 0) return true;

        }

        return false;
    }

}