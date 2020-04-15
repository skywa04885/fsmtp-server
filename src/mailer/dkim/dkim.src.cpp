/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "dkim.src.hpp"

namespace Fannst::FSMTPServer::DKIM {
    /**
     * Formats an array of signature parts, into the final signature
     * @param parts
     * @param ret
     * @return
     */
    int formatSignature(const char *parts[FANNST_DKIM_TOTAL_PARTS], char *ret) {
        return 0;
    }

    /**
     * Builds an signature part
     * @param key
     * @param kv
     * @return
     */
    char *buildSigPart(const char *key, const char *kv)
    {
        // Allocates the memory
        char *ret = reinterpret_cast<char *>(malloc(
                strlen(&key[0]) + strlen(&kv[0]) + 1));
        ret[0] = '\0';

        // Starts appending the results
        strcat(&ret[0], &key[0]);       // Appends the key
        strcat(&ret[0], "=");           // Appends the equals sign
        strcat(&ret[0], &kv[0]);        // Appends the key value

        // Returns the result
        return ret;
    }

    /**
     * Signs an MIME message
     * @param raw
     * @param config
     * @return
     */
    int sign(const char *raw, char *sigRet, const DKIMHeaderConfig *config)
    {
        /*
         * 1. Create already-known values
         * 2. Canonicalize body and create body-hash
         * 3. Canonicalize headers and create the signature
         * 4. Sign the signature
         * 5. Join the headers with max len defined in FANNST_DKIM_LINE_MAX_LEN
         */

        // ----
        // Creates the logger
        // ----

        DEBUG_ONLY(Logger::Console print(Logger::Level::LOGGER_DEBUG, "DKIM Signer"))

        // ----
        // Combines all the parts into the final sig parts array
        // ----

        char *sigParts[FANNST_DKIM_TOTAL_PARTS] {nullptr};

        // Prepares the expire, and sign date strings
        char sTime[24];
        char xTime[24];
        sprintf(&sTime[0], "%lu", config->d_SignDate);
        sprintf(&xTime[0], "%lu", config->d_ExpireDate);

        // Sets the already known parts, into the array
        sigParts[0] = buildSigPart("d", config->d_Domain);
        sigParts[1] = buildSigPart("s", config->d_KeyS);
        sigParts[2] = buildSigPart("t", sTime);
        sigParts[3] = buildSigPart("x", xTime);

        switch(config->d_Ago)
        {
            case DKIMCanAlgorithms::DCA_RELAXED_RELAXED:
            {
                sigParts[4] = buildSigPart("c", "relaxed/relaxed");
                break;
            }
            case DKIMCanAlgorithms::DCA_RELAXED_SIMPLE:
            {
                sigParts[4] = buildSigPart("c", "relaxed/simple");
                break;
            }
            case DKIMCanAlgorithms::DCA_SIMPLE_RELAXED:
            {
                sigParts[4] = buildSigPart("c", "simple/relaxed");
                break;
            }
            case DKIMCanAlgorithms::DCA_SIMPLE_SIMPLE:
            {
                sigParts[4] = buildSigPart("c", "simple/simple");
                break;
            }
        }

        sigParts[5] = buildSigPart("q", "dns/txt");
        sigParts[6] = buildSigPart("v", "1");
        sigParts[7] = buildSigPart("a", "rsa-sha256");

        // ----
        // Performs debug print, if enabled
        // ----

        // Prints the parts
        #ifdef DEBUG
        print << "Raw parts: " << Logger::ConsoleOptions::ENDL;
        for (std::size_t i = 0; i < FANNST_DKIM_TOTAL_PARTS; i++)
        {
            if (sigParts[i] != nullptr) print << " - Sigparts [" << i << "]: " << sigParts[i] << Logger::ConsoleOptions::ENDL;
        }
        #endif

        // ----
        // Parses the message body, so we can create the body-hash
        // ----

        DEBUG_ONLY(print << "Started body splitter .." << Logger::ConsoleOptions::ENDL)

        char *headerRet = nullptr;
        char *bodyRet = nullptr;

        parseMimeMessage(raw, &headerRet, &bodyRet);

        DEBUG_ONLY(print << " - Headers: " << headerRet << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(print << " - Body: " << bodyRet << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(print << "Finished body splitter !" << Logger::ConsoleOptions::ENDL)

        // ----
        // Starts the canonicalization of the headers
        // ----

        DEBUG_ONLY(print << "Starting header canonicalization ..." << Logger::ConsoleOptions::ENDL)

        // Checks which algorithm we will use
        if (config->d_Ago == DKIMCanAlgorithms::DCA_RELAXED_SIMPLE ||
            config->d_Ago == DKIMCanAlgorithms::DCA_RELAXED_RELAXED)
        { // Relaxed
            char *canHeadersRet = nullptr;

            canonicalizeHeadersRelaxed(headerRet, &canHeadersRet);
        } else
        { // Simple
            // TODO: Implement simple algorithm
        }

        // ----
        // Starts the canonicalization of the body
        // ----

        return 0;
    }
}