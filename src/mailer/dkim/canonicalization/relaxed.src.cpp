/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "../dkim.src.hpp"

namespace Fannst::FSMTPServer::DKIM {
    /**
     * Removes unwanted whitespace
     * @param a
     * @param aLen
     */
    inline void cleanWhitespace(char *a, std::size_t aLen)
    {
        // Copies the string
        char *b = reinterpret_cast<char *>(malloc(aLen));
        memcpy(&b[0], &a[0], aLen);
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
        }

        return false;
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

    /**
     * Canonicalizes headers using the relaxed method
     * @param raw
     * @param ret
     */
    void canonicalizeHeadersRelaxed(const char *raw, char **ret)
    {
        /*
         * 1. Make all keys lowercase
         * 2. Remove whitespace after key
         * 3. Idk yet .....
         */

        // ----
        // Allocates 1 byte of memory as starting string in ret
        // ----

        std::size_t retBufferSize = 1;
        *ret = reinterpret_cast<char *>(malloc(1));
        (*ret)[0] = '\0';

        // ----
        // Prepares the tokenizer
        // ----

        std::size_t size2cpy = strlen(&raw[0]);
        char *rawC = reinterpret_cast<char *>(malloc(size2cpy));
        memcpy(&rawC[0], &raw[0], size2cpy);

        // Creates the tokenizer, and grabs first token
        char *tok = strtok(&rawC[0], "\r\n");
        int i;
        int splitPos;

        // ----
        // Starts looping over the headers
        // Note: We will use cRaw as result, because it will overcome overhead
        // ----

        while (tok != nullptr)
        {
            // Gets the position of ':'
            splitPos = findChar(&tok[0], ':');
            // Converts the chars to lower case
            for (i = 0; i < splitPos; i++) tok[i] = tolower(tok[i]);

            // Checks if it should be used
            if (!shouldUseHeader(&tok[0]))
            {
                // Goes to the next token
                tok = strtok(nullptr, "\r\n");

                // Skips
                continue;
            }

            // Resize the buffer
            retBufferSize += strlen(&tok[0]) + 1;
            // Reallocates the memory
            *ret = reinterpret_cast<char *>(realloc(&(*ret)[0], retBufferSize));

            // Adds the strings
            tok[splitPos] = '\0';
            strcat(&(*ret)[0], &tok[0]);
            strcat(&(*ret)[0], ":");
            strcat(&(*ret)[0], &tok[splitPos+2]);
            strcat(&(*ret)[0], "\r\n");

            // Goes to the next token
            tok = strtok(nullptr, "\r\n");
        }

        // Frees the memory
        free(rawC);
    }

    /**
     * Canonicalizes body using the relaxed method
     * @param raw
     * @param ret
     */
    void canonicalizeBodyRelaxed(const char *raw, char **ret)
    {
        // ----
        // Allocates 1 byte of memory as starting string in ret
        // ----

        std::size_t retBufferSize = 1;
        *ret = reinterpret_cast<char *>(malloc(1));
        (*ret)[0] = '\0';

        // Frees the memory
        free(rawC);
    }
}