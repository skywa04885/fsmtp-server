/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "../dkim.src.hpp"

namespace Fannst::FSMTPServer::DKIM::OpenSSL {
    /**
     * Performs SHA256 hash, and returns the base 64 version
     * @param raw
     * @param hRet
     */
    void sha256base64(const char *raw, char **hRet)
    {
        // ----
        // Performs the hash
        // ----

        // Creates the context
        SHA256_CTX ctx;

        // Inits the hash
        if (SHA256_Init(&ctx) <= 0)
        {
            PREP_ERROR("SHA256 Hash", "Could not perform SHA256_Init")
        }

        // Updates the hash
        if (SHA256_Update(&ctx, &raw[0], strlen(&raw[0])) <= 0)
        {
            PREP_ERROR("SHA256 Hash", "Could not perform SHA256_Update")
        }

        // Gets the hash result
        auto *digest = reinterpret_cast<unsigned char *>(malloc(SHA256_DIGEST_LENGTH + 1));
        if (SHA256_Final(&digest[0], &ctx) <= 0)
        {
            PREP_ERROR("SHA256 Hash", "Could not perform SHA256_Final")
        }

        // ----
        // Gets the base64 string
        // ----
        digest[SHA256_DIGEST_LENGTH] = '\0';
        *hRet = const_cast<char *>(Base64::encode(reinterpret_cast<const char *>(digest)));

        // ----
        // Frees the memory
        // ----

        free(digest);
    }
}