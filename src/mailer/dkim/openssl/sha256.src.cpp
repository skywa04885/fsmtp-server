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
        auto *digest = reinterpret_cast<unsigned char *>(malloc(SHA256_DIGEST_LENGTH));
        if (SHA256_Final(&digest[0], &ctx) <= 0)
        {
            PREP_ERROR("SHA256 Hash", "Could not perform SHA256_Final")
        }

        // ----
        // Gets the Base64 encoded value
        // ----

        BIO *bio, *b64;
        BUF_MEM *bufMem;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

        BIO_write(bio, digest, SHA256_DIGEST_LENGTH);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufMem);
        BIO_set_close(bio, BIO_NOCLOSE);
        BIO_free_all(bio);

        // ----
        // Stores the result
        // ----

        *hRet = (*bufMem).data;

        // ----
        // Frees the memory
        // ----

        free(digest);
    }
}