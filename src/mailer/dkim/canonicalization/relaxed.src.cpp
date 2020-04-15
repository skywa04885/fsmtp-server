/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "../dkim.src.hpp"

namespace Fannst::FSMTPServer::DKIM {
    inline std::size_t findChar(char *a)
    {
        // Creates the result
        std::size_t res = 0;

        // Infinite loop
        for (;;)
        {
            // Checks if we should break
            if (*a == '\0') break;

            // Goes to the next char in memory
            a++;
            res++;
        }

        // Returns the index of the char
        return res;
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

        std::size_t size2copy = strlen(&raw[0]);
        char *cRaw = reinterpret_cast<char *>(malloc(size2copy));
        memcpy(&cRaw[0], &raw[0], size2copy);

        // Creates the tokenizer, and grabs first token
        char *tok = strtok(&cRaw[0], "\r\n");

        // ----
        // Starts looping over the headers
        // Note: We will use cRaw as result, because it will overcome overhead
        // ----

        while (tok != nullptr)
        {
            // Splits the string on ':'

            // Goes to the next token
            tok = strtok(nullptr, "\r\n");
        }
    }
}