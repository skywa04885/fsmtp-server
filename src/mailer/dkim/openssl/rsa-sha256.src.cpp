/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "../dkim.src.hpp"

namespace Fannst::FSMTPServer::DKIM::OpenSSL {
    /**
     * Performs RSA-SHA256 Signature, and returns the base 64 version
     * @param raw
     * @param hRet
     */
    int rsaSha256genSig(const char *raw, const char *pKeyFile, char **hRet)
    {
        // ----
        // Reads the private key
        // ----

        // Opens the file
        FILE *pKey = fopen(pKeyFile, "rt");
        if (!pKey)
        {
            PREP_ERROR("Could not open PEM file", pKeyFile);
            return -1;
        }

        // Reads the key
        RSA *rsa = PEM_read_RSAPrivateKey(pKey, nullptr, nullptr, nullptr);
        if (!rsa)
        {
            PREP_ERROR("Could not read RSA Private Key", pKeyFile);
            return -1;
        }

        // Closes the file
        fclose(pKey);

        // ----
        // Starts signing
        // ----

        // Creates the sign context
        EVP_MD_CTX *rsaSignContext = EVP_MD_CTX_new();
        // Creates the private keu
        EVP_PKEY *evpPkey = EVP_PKEY_new();
        // Assigns the RSA key
        EVP_PKEY_assign_RSA(evpPkey, rsa);

        // Initializes the signer
        if (EVP_DigestSignInit(rsaSignContext, nullptr, EVP_sha256(), nullptr, evpPkey) <= 0)
        {
            PREP_ERROR("Could not initialize signer", pKeyFile);
            return -1;
        }

        // Updates the signer
        if (EVP_DigestSignUpdate(rsaSignContext, &raw[0], strlen(&raw[0])) <= 0)
        {
            PREP_ERROR("Could not update signer", pKeyFile);
            return -1;
        }

        // Gets the signed content length
        std::size_t msgLen;
        if (EVP_DigestSignFinal(rsaSignContext, nullptr, &msgLen) <= 0)
        {
            PREP_ERROR("Could not get signer result len", pKeyFile);
            return -1;
        }

        // Allocates memory for result
        auto *msgRet = reinterpret_cast<unsigned char *>(malloc(msgLen));

        // Gets the result
        if (EVP_DigestSignFinal(rsaSignContext, msgRet, &msgLen) <= 0)
        {
            PREP_ERROR("Could not get signer result len", pKeyFile);
            return -1;
        }

        // Cleans
        EVP_MD_CTX_free(rsaSignContext);

        // ----
        // Gets the Base64 encoded value
        // ----

        BIO *bio, *b64;
        BUF_MEM *bufMem;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

        BIO_write(bio, msgRet, msgLen);
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

        free(msgRet);
    }
}