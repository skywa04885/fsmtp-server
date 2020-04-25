/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "dkim.src.hpp"

namespace Fannst::FSMTPServer::DKIM {
    /**
     * Builds an signature part
     * @param key
     * @param kv
     * @return
     */
    void buildSigPart(const char *key, const char *kv, char **ret)
    {
        // ----
        // Handles the allocation
        // ----

        // Free old memory, if pointer is not nullptr
        if (*ret != nullptr) free(*ret);

        // Allocates required size for operation
        *ret = reinterpret_cast<char *>(malloc(
                ALLOC_CAS_STRING(strlen(&key[0]) + strlen(&kv[0]), 1)
                ));

        // Sets the first char to '\0' because it is an null terminated string
        *ret[0] = '\0';

        // -----
        // Performs the string operations
        // -----

        // Starts appending the results
        strcat(&(*ret)[0], &key[0]);       // Appends the key
        strcat(&(*ret)[0], "=");           // Appends the equals sign
        strcat(&(*ret)[0], &kv[0]);        // Appends the key value
    }

    /**
     * Generates th headers field
     * @param raw
     * @param ret
     */
    void generateHeadersField(const char *raw, char **ret)
    {
        char *tok = nullptr, *rawC = nullptr;
        int splitPos, i;
        std::size_t retSize, tempSize;

        // ----
        // Prepares the ret value
        // ----

        // Sets the buffer size
        retSize = 1;

        // Allocates the required memory
        *ret = reinterpret_cast<char *>(malloc(1));
        PREP_ALLOCATE_INVALID(*ret);

        // Sets the null termination char
        (*ret)[0] = '\0';

        // ----
        // Prepares the tokenizer
        // ----

        // Gets the length of raw
        tempSize = strlen(&raw[0]);

        // Allocates memory for the copy
        rawC = reinterpret_cast<char *>(malloc(ALLOCATE_NULL_TERMINATION(tempSize)));

        // Copies the memory
        memcpy(&rawC[0], &raw[0], tempSize + 1);

        // Creates the tokenizer
        tok = strtok(&rawC[0], "\r\n");

        // Starts looping
        while (tok != nullptr)
        {
            // ----
            // Prepares
            // ----

            // Gets the position of ':'
            splitPos = findChar(&tok[0], ':');

            // Converts the chars to lower case
            for (i = 0; i < splitPos; i++) tok[i] = static_cast<char>(tolower(tok[i]));

            // ----
            // Checks if we should use the header
            // ----

            // Checks if it should be used
            if (!shouldUseHeader(&tok[0]))
            {
                // Goes to the next token
                tok = strtok(nullptr, "\r\n");

                // Skips
                continue;
            }

            // Sets the split part
            tok[splitPos] = '\0';

            // ----
            // Resize the buffer
            // ----

            // Sets the new buffer size
            retSize += splitPos + 1;

            // Reallocates the buffer
            *ret = reinterpret_cast<char *>(realloc(&(*ret)[0], retSize));
            PREP_ALLOCATE_INVALID(*ret);

            // ----
            // Concats the data into it
            // ----

            strcat(&(*ret)[0], &tok[0]);
            strcat(&(*ret)[0], ":");

            // Gets the next token
            tok = strtok(nullptr, "\r\n");
        }

        // ----
        // Frees the memory
        // ----

        free(rawC);

        // ----
        // Finishes
        // ----

        // Removes the ':' at the end
        (*ret)[retSize-2] = '\0';
    }


    /**
     * Signs an MIME message
     * @param raw
     * @param config
     * @return
     */
    int sign(const char *raw, char **sigRet, const DKIMHeaderConfig *config)
    {
        char *sigParts[FANNST_DKIM_TOTAL_PARTS]{nullptr};

        char sTime[24];                     // Contains the sign time
        char xTime[24];                     // Contains the expire time

        char *headerRet = nullptr;          // Contains the header separated from the body
        char *bodyRet = nullptr;            // Contains the body separated from the headers
        char *sig = nullptr;                // Contains the temp dkim headers
        char *canHeadersRet = nullptr;      // Contains the canonicalized headers
        char *bodyHash = nullptr;           // Contains the body hash
        char *headerSigRet = nullptr;       // Contains the signature itself
        char *canBodyRet = nullptr;         // Contains the canonicalized body

        char *headerPartsRet = nullptr;     // The parts of the headers, which will be canonicalized

        // Will be used to seperate the headers from the first generated DKIM header
        std::size_t headerEndIndex;
        std::size_t tCount;

        // ----
        // Creates the logger
        // ----

        DEBUG_ONLY(Logger::Console print(Logger::Level::LOGGER_DEBUG, "DKIM Signer"))

        // ----
        // Adds the domain, key selector, sign date and expire date to the key pairs
        // ----

        // Prepares the expire, and sign date strings
        sprintf(&sTime[0], "%lu", config->d_SignDate);
        sprintf(&xTime[0], "%lu", config->d_ExpireDate);

        // Sets the already known parts, into the array
        buildSigPart("d", config->d_Domain, &sigParts[0]);
        buildSigPart("s", config->d_KeyS, &sigParts[1]);
        buildSigPart("t", sTime, &sigParts[2]);
        buildSigPart("x", xTime, &sigParts[3]);

        // ----
        // Adds the algorithm pair to the header
        // ----

        switch(config->d_Ago)
        {
            case DKIMCanAlgorithms::DCA_RELAXED_RELAXED:
            {
                buildSigPart("c", "relaxed/relaxed", &sigParts[4]);
                break;
            }
            case DKIMCanAlgorithms::DCA_RELAXED_SIMPLE:
            {
                buildSigPart("c", "relaxed/simple", &sigParts[4]);
                break;
            }
            case DKIMCanAlgorithms::DCA_SIMPLE_RELAXED:
            {
                buildSigPart("c", "simple/relaxed", &sigParts[4]);
                break;
            }
            case DKIMCanAlgorithms::DCA_SIMPLE_SIMPLE:
            {
                buildSigPart("c", "simple/simple", &sigParts[4]);
                break;
            }
        }

        // ----
        // Adds the query type, version and algorithm for signing and hasing
        // ----

        buildSigPart("q", "dns/txt", &sigParts[5]);
        buildSigPart("v", "1", &sigParts[6]);
        buildSigPart("a", "rsa-sha256", &sigParts[7]);

        // ----
        // Prints the signature parts, if debug enabled
        // ----

        // Prints the parts
        #ifdef DEBUG
        print << "Raw parts: " << Logger::ConsoleOptions::ENDL;
        for (std::size_t i = 0; i < FANNST_DKIM_TOTAL_PARTS; i++)
            if (sigParts[i] != nullptr) print << " - sigParts [" << i << "]: " << sigParts[i] << Logger::ConsoleOptions::ENDL;
        #endif

        // ----
        // Parses both the header and body from the email
        // ----

        // Parses the message, allocates memory for the pointers
        parseMimeMessage(raw, &headerRet, &bodyRet);

        // Prints the update to the console, for debug purposes
        DEBUG_ONLY(print << "Body and Headers successfully parsed from message:" << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(print << " - Headers: " << headerRet << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(print << " - Body: " << bodyRet << Logger::ConsoleOptions::ENDL)

        // ----
        // Starts the Canonicalization of the body
        // ----

        // Canonicalizes the message body using the relaxed algoritm
        canonicalizeBodyRelaxed(&bodyRet[0], &canBodyRet);

        // Prints the updates to the debug console
        DEBUG_ONLY(print << "Finishes body canonicalization: " << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(std::cout << '\'' << canBodyRet << "\'" << std::endl)

        // ----
        // Calculates the SHA256 Hash of the message body
        // ----

        // Calculates the hash value
        OpenSSL::sha256base64(&canBodyRet[0], &bodyHash);

        // Prints the body hash for debug purposes
        DEBUG_ONLY(print << "Body hash created: " << bodyHash << Logger::ConsoleOptions::ENDL)

        // ----
        // Calculates the h field
        // ----

        // Generates the header fields
        generateHeadersField(&headerRet[0], &headerPartsRet);

        // Sets the h field in the parts
        buildSigPart("h", headerPartsRet, &sigParts[8]);

        // ----
        // Adds the header fields and body hash to the sigParts
        // ----

        buildSigPart("bh", bodyHash, &sigParts[9]);

        // ----
        // Generates an DKIM-Header without the signature itself, why ?
        // Because the RFC guidelines specify it is required for an valid hash ;)
        // ----

        // Gets the current length of the headers, which will later be used to set the zero termination char
        headerEndIndex = strlen(&headerRet[0]);

        // Adds the signature part itself, but without an actual value
        buildSigPart("b", "", &sigParts[10]);

        // Creates the temp dkim header, which will be passed into the algorithm
        formatSignature(sigParts, &sig, false);

        // Allocates the memory, with the required DKIM Signature
        headerRet = reinterpret_cast<char *>(realloc(&headerRet[0], ALLOC_CAS_STRING(strlen(&headerRet[0]),
                                                                                     strlen(&sig[0]) + 2 + 16)));

        // Appends the result, with <CRLF>
        strcat(&headerRet[0], "DKIM-Signature: ");
        strcat(&headerRet[0], &sig[0]);
        strcat(&headerRet[0], "\r\n");

        // ----
        // Starts the pre-processing of the headers
        // ----

        // Checks with which algorithm the headers will be processed
        if (config->d_Ago == DKIMCanAlgorithms::DCA_RELAXED_SIMPLE ||
            config->d_Ago == DKIMCanAlgorithms::DCA_RELAXED_RELAXED)
        { // Relaxed

            // Canonicalizes
            canonicalizeHeadersRelaxed(headerRet, &canHeadersRet);
        } else
        { // Simple
            // TODO: Implement simple algorithm
        }

        // ----
        // Generates the signature ( Final stage )
        // ----

        // Prints the canonicalized headers
        DEBUG_ONLY(print << "Finished header Canonicalization: " << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(std::cout << canHeadersRet << std::endl)

        // Creates the actual signature
        OpenSSL::rsaSha256genSig(canHeadersRet, config->d_PKey, &headerSigRet);

        // Prints the debug stuff
        DEBUG_ONLY(print << "Generated signature: " << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(std::cout << "\033[36m'" << headerSigRet << "'\033[0m" << std::endl)

        // ----
        // Adds the signature to the DKIM header parts
        // ----

        buildSigPart("b", headerSigRet, &sigParts[10]);

        // ----
        // Formats the final headers, and stores them inside sig
        // ----

        // Frees the old signature
        free(sig);

        // Performs the formatting with newline chars
        formatSignature(sigParts, &sig, true);

        // Prints the signature for debug purposes
        DEBUG_ONLY(print << "Created signature: " << Logger::ConsoleOptions::ENDL)
        DEBUG_ONLY(std::cout << sig << std::endl)

        // ----
        // Restores the original headers, so we can append our DKIM-Header without any duplications
        // ----

        // Sets the zero termination character
        headerRet[headerEndIndex] = '\0';

        // ----
        // Appends the signature to the result
        // ----

        // Allocates the final memory for the signature
        *sigRet = reinterpret_cast<char *>(malloc(
                ALLOC_CAS_STRING(strlen(&sig[0]) + strlen(&bodyRet[0]) + strlen(&headerRet[0]), 6 + 16)
                ));

        // Adds the zero termination character
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

        // Frees the char pointers
        free(headerRet);
        free(bodyRet);
        free(sig);
        free(canHeadersRet);
        free(bodyHash);
        free(headerSigRet);
        free(canBodyRet);
        free(headerPartsRet);

        // Frees our array
        for (BYTE i = 0; i < FANNST_DKIM_TOTAL_PARTS; i++) free(sigParts[i]);

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
        std::size_t retBufferSize;
        std::size_t cLen;
        std::size_t cStringLen;
        std::size_t tStringSize;
        std::size_t tCount;

        char *curr = nullptr;

        bool clearTempString;

        // ----
        // Allocates memory for ret
        // ----

        retBufferSize = 1;
        *ret = reinterpret_cast<char *>(malloc(1));
        (*ret)[0] = '\0';

        // ----
        // Loops over the parts
        // ----

        cLen = 0;
        clearTempString = false;

        for (char i = 0; i < FANNST_DKIM_TOTAL_PARTS; i++)
        {
            // Gets the current part
            curr = parts[i];
            cStringLen = strlen(&curr[0]);

            // ----
            // Checks if the line itself is longer then max len, if so format it first
            // ----

            if (cStringLen > FANNST_DKIM_MAX_SIG_LL && nl)
            { // Pre-process the data

                // Allocates the memory
                tStringSize = strlen(&curr[0]);
                char *tString = reinterpret_cast<char *>(malloc(tStringSize));
                tString[0] = '\0';

                // Gets the memory address of curr without the A <EQ>
                char *c = &curr[0];

                // Starts looping
                tCount = 0;
                while (*c != '\0')
                {
                    // Checks if <CRLF> needs to be added
                    if (tCount > FANNST_DKIM_MAX_SIG_LL)
                    {
                        // Resize the buffer
                        tStringSize += 2 + 5;
                        tString = reinterpret_cast<char *>(realloc(&tString[0], tStringSize));

                        // Reset the count
                        tCount = 0;

                        // Concats the strings
                        strcat(&tString[0], "\r\n     ");
                    }

                    // Concat the string
                    strncat(&tString[0], c, 1);

                    // Goes to the next char
                    c++;
                    tCount++;
                }

                // Stores the result
                curr = tString;
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

                // If required, clears the temp string
                if (clearTempString) free(curr);

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