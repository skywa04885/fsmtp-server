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
    char *cleanWhitespace(const char *a, std::size_t aLen)
    {
        // ----
        // Prepares the memory copy
        // ----

        // Reserves the memory
        char *tStr = reinterpret_cast<char *>(malloc(aLen));
        // Resets the len so we can use it as an coutner
        aLen = 0;

        // ----
        // Starts looping
        // ----

        bool lww = false;
        while (*a != '\0')
        {
            // Checks if it is whitespace
            if (*a == ' ')
            {
                // If previous was whitespace, skip char
                if (lww)
                {
                    a++;
                    continue;
                }

                lww = true;
            } else lww = false;

            // Appends the char
            tStr[aLen] = *a;

            // Increments the indexes
            aLen++;
            a++;
        }

        tStr[aLen] = '\0';

        // Returns the result
        return tStr;
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

        // ----
        // Removes double whitespace
        // ----

        // Gets an version without whitespace
        char *raw2 = cleanWhitespace(&raw[0], strlen(&raw[0]));

        // ----
        // Finishes the data by
        // 1. Remove whitespace at end of lines
        // 2. Remove all empty lines at end of body
        // ----

        // Creates the tokenizer, splis at '\r' so we can check if line empty
        char *tok = strtok(&raw2[0], "\r");

        // Loops while tokens available
        std::size_t i = 0;
        std::size_t iLastRealContent = 0;
        std::size_t cSize;
        while (tok != nullptr)
        {
            // Gets the token size
            cSize = strlen(&tok[0]);

            // ----
            // Removes the not wanted space
            // ----

            if (tok[cSize-1] == ' ')
            {
                // Removes the last char by setting it to '\0'
                tok[cSize-1] = '\0';
                // Removes one from the size, so the buffer will allocate one less
                cSize--;
            }

            // ----
            // Stores the data
            // ----

            // Increases buffer size
            retBufferSize += cSize + 2;
            *ret = reinterpret_cast<char *>(realloc(&(*ret)[0], retBufferSize));

            // If first row, add whole string, else without '\n'
            if (i == 0) strcat(&(*ret)[0], &tok[0]);
            else strcat(&(*ret)[0], &tok[1]);

            // Adds the '\r\n'
            strcat(&(*ret)[0], "\r\n");

            // ----
            // Checks if the line was empty, if not store index
            // ----

            if (tok[1]) iLastRealContent = i;

            // ----
            // Goes to the next item
            // ----

            // Increments the index
            i++;
            // Goes to the next token
            tok = strtok(nullptr, "\r");
        }

        // ----
        // Removes the not required empty lines
        // ----

        // Removes the last real content from i, to determine where we are going to stop the string
        i -= iLastRealContent;

        // Removes one because one of the last is required
        i -= 1;

        // Multiplies i by two, so we get the amount of chars to remove from the string end
        i *= 2;

        // Substracts it from the string
        (*ret)[strlen(&(*ret)[0]) - i] = '\0';

        // ----
        // Free memory
        // ----

        free(raw2);
    }
}