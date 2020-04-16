/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <fannst_libcompose.hpp>
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
    int sign(const char *raw, char **sigRet, const DKIMHeaderConfig *config)
    {
        std::cout << raw << std::endl;

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
        // Starts the canonicalization of the body
        // ----

        char *canBodyRet = nullptr;
        DEBUG_ONLY(print << "Starting body canonicalization ..." << Logger::ConsoleOptions::ENDL)

        // Canonicalizes the section
        canonicalizeBodyRelaxed(&bodyRet[0], &canBodyRet);
        DEBUG_ONLY(std::cout << '\'' << bodyRet << "\'" << std::endl)

        // Calculates the hash value
        char *bodyHash = nullptr;
        OpenSSL::sha256base64(&canBodyRet[0], &bodyHash);
        DEBUG_ONLY(std::cout << bodyHash << std::endl)

        // Adds the part to the final array
        sigParts[8] = buildSigPart("h", "date:message-id:from:to:subject");
        sigParts[9] = buildSigPart("bh", bodyHash);

        // ----
        // Before header canonicalization, add the dkim without b data
        // ----

        // Gets the current header length, so we can leter subtract the last not required part
        std::size_t hLenWithoutDKIM = strlen(&headerRet[0]);

        // Adds the signature part
        sigParts[10] = buildSigPart("b", "");

        // Creates the temp res
        char *sig = nullptr;
        formatSignature(sigParts, &sig, false);

        // Appends to the result headers
        {
            // Gets the current length
            std::size_t tl = strlen(&headerRet[0]);

            // Resizes the headers
            tl += strlen(&sig[0]) + 2 + 16;
            headerRet = reinterpret_cast<char *>(realloc(&headerRet[0], tl));

            // Appends the result, with <CRLF>
            strcat(&headerRet[0], "DKIM-Signature: ");
            strcat(&headerRet[0], &sig[0]);
            strcat(&headerRet[0], "\r\n");
        }

        // ----
        // Starts the canonicalization of the headers
        // ----

        char *canHeadersRet = nullptr;
        DEBUG_ONLY(print << "Starting header canonicalization ..." << Logger::ConsoleOptions::ENDL)

        // Checks which algorithm we will use
        if (config->d_Ago == DKIMCanAlgorithms::DCA_RELAXED_SIMPLE ||
            config->d_Ago == DKIMCanAlgorithms::DCA_RELAXED_RELAXED)
        { // Relaxed
            canonicalizeHeadersRelaxed(headerRet, &canHeadersRet);
        } else
        { // Simple
            // TODO: Implement simple algorithm
        }

        DEBUG_ONLY(print << "Finished header canonicalization !" << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(std::cout << canHeadersRet << std::endl)

        // Signs the headers
        char *headerSigRet = nullptr;
        OpenSSL::rsaSha256genSig(canHeadersRet, config->d_PKey, &headerSigRet);

        DEBUG_ONLY(print << "Generated signature !" << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(std::cout << "\033[36m'" << headerSigRet << "'\033[0m" << std::endl)

        // Sets the parameter
        sigParts[10] = buildSigPart("b", headerSigRet);

        // ----
        // Creates the result
        // ----

        formatSignature(sigParts, &sig, true);

        DEBUG_ONLY(print << "Created signature: " << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(std::cout << sig << std::endl)

        // ----
        // Restores the old headers
        // ----

        headerRet[hLenWithoutDKIM] = '\0';

        // ----
        // Appends the signature to the result
        // ----

        *sigRet = reinterpret_cast<char *>(malloc(
                strlen(&sig[0]) + strlen(&bodyRet[0]) + strlen(&headerRet[0]) + 6 + 16
                ));
        (*sigRet)[0] = '\0';

        // Appends the strings
        strcat(&(*sigRet)[0], &headerRet[0]);
        strcat(&(*sigRet)[0], "DKIM-Signature: ");
        strcat(&(*sigRet)[0], &sig[0]);
        strcat(&(*sigRet)[0], "\r\n\r\n");
        strcat(&(*sigRet)[0], &bodyRet[0]);

        // ----
        // Frees the memory
        // ----

        free(canHeadersRet);
        free(canBodyRet);
        free(bodyHash);
        free(sig);
        free(headerSigRet);

        return 0;
    }

    /**
     * Formats an array of signature parts, into the final signature
     * @param parts
     * @param ret
     * @return
     */
    int formatSignature(char *parts[FANNST_DKIM_TOTAL_PARTS], char **ret, bool nl)
    {
        // Allocates result memory
        std::size_t retBufferSize = 1;
        *ret = reinterpret_cast<char *>(malloc(1));
        (*ret)[0] = '\0';

        // Starts looping over the parts
        char *curr = nullptr;
        std::size_t cLen = 0;
        std::size_t cStringLen;
        bool clearTempString = false;
        for (std::size_t i = 0; i < FANNST_DKIM_TOTAL_PARTS; i++)
        {
            // Gets the current part
            curr = parts[i];
            // Gets the len
            cStringLen = strlen(&curr[0]);

            // ----
            // Checks if the line itself is longer then max len, if so format it first
            // ----

            if (cStringLen > FANNST_DKIM_MAX_SIG_LL && nl)
            { // Pre-process the data

                // Allocates the memory
                std::size_t tStringSize = strlen(&curr[0]);
                char *tString = reinterpret_cast<char *>(malloc(tStringSize));
                tString[0] = '\0';

                // Gets the memory address of curr without the A <EQ>
                char *c = &curr[0];

                // Starts looping
                std::size_t ct = 0;
                while (*c != '\0')
                {
                    // Checks if <CRLF> needs to be added
                    if (ct > FANNST_DKIM_MAX_SIG_LL)
                    {
                        // Resize the buffer
                        tStringSize += 2 + 5;
                        tString = reinterpret_cast<char *>(realloc(&tString[0], tStringSize));

                        ct = 0;

                        // Concats the strings
                        strcat(&tString[0], "\r\n     ");
                    }

                    // Concat the string
                    strncat(&tString[0], c, 1);

                    // Goes to the next char
                    c++;
                    // Increments the char
                    ct++;
                }

                // Stores the result
                curr = &tString[0];
                cStringLen = strlen(&tString[0]);
                clearTempString = true;
            }

            // ----
            // Checks if it fits in the line
            // ----

            if (cLen + cStringLen > FANNST_DKIM_MAX_SIG_LL && nl)
            { // Append on new line

                // Sets the result buffer size
                retBufferSize += cStringLen + 8;
                *ret = reinterpret_cast<char *>(realloc(&(*ret)[0], retBufferSize));

                // Appends the strings
                strcat(&(*ret)[0], "\r\n    ");
                strcat(&(*ret)[0], &curr[0]);
                strcat(&(*ret)[0], "; ");

                // Sets the cLen to zero
                cLen = 0;

                continue;
            }

            // ----
            // Appends on same line
            // ----

            // Sets the result buffer size
            retBufferSize += cStringLen + 2;
            *ret = reinterpret_cast<char *>(realloc(&(*ret)[0], retBufferSize));

            // Appends the strings
            strcat(&(*ret)[0], &curr[0]);
            strcat(&(*ret)[0], "; ");

            // Increments the length
            cLen += cStringLen;

            // If required, clears the temp string
            if (clearTempString) free(curr);
        }

        // ----
        // Removes the last "; "
        // ----

        (*ret)[strlen(&(*ret)[0]) - 2] = '\0';

        return 0;
    }
}