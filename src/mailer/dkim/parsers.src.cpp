/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "dkim.src.hpp"

namespace Fannst::FSMTPServer::DKIM {
    /**
     * Splits MIME message into two sections
     * @param raw
     * @param headerRet
     * @param bodyRet
     * @return
     */
    int parseMimeMessage(const char *raw, char **headerRet, char **bodyRet)
    {
        // ----
        // Prepares the buffers for the results
        // ----

        // Prepares the headers return
        *headerRet = reinterpret_cast<char *>(malloc(1));
        (*headerRet)[0] = '\0';
        std::size_t headersBufferSize = 1;

        // Prepares the body return
        *bodyRet = reinterpret_cast<char *>(malloc(1));
        (*bodyRet)[0] = '\0';
        std::size_t bodyBufferSize = 1;

        // ----
        // Starts looping over the message, by tokenizing on "\r\n"
        // ----

        // Prepares the looping variables
        bool bodyHit = false;

        // Copies the raw data, so we can process it
        std::size_t size2cpy = strlen(&raw[0]);
        char *rawC = reinterpret_cast<char *>(malloc(size2cpy));
        memcpy(&rawC[0], &raw[0], size2cpy);

        // Prepares the tokenizer, we split at \r because we will need to chech
        // if empty
        char *tok = strtok(&rawC[0], "\r");
        char i = 0;

        // Creates the tokenizing loop
        while (tok != nullptr)
        {
            // Checks if the body is hit, and updates accordingly
            if (!bodyHit && !tok[1])
            {
                // Sets body hit to true
                bodyHit = true;

                // Goes to the next token
                tok = strtok(nullptr, "\r");
                // Increments i
                i++;

                // Skips the code
                continue;
            }

            // Checks to which buffer we need to append
            if (bodyHit)
            { // Is body

                // Sets the new buffer size, without the '\n', with "\r\n"
                bodyBufferSize += strlen(&tok[1]) + 2;

                // Reallocates the memory
                *bodyRet = reinterpret_cast<char *>(realloc(&(*bodyRet)[0], bodyBufferSize));

                // If first row, add whole string, else without '\n'
                if (i == 0) strcat(&(*bodyRet)[0], &tok[0]);
                else strcat(&(*bodyRet)[0], &tok[1]);

                // Adds the '\r\n'
                strcat(&(*bodyRet)[0], "\r\n");
            } else
            { // Is headers

                // Sets the new buffer size, without the '\n', with "\r\n"
                headersBufferSize += strlen(&tok[1]) + 2;

                // Reallocates the memory
                *headerRet = reinterpret_cast<char *>(realloc(&(*headerRet)[0], headersBufferSize));

                // If first row, add whole string, else without '\n'
                if (i == 0) strcat(&(*headerRet)[0], &tok[0]);
                else strcat(&(*headerRet)[0], &tok[1]);

                // Adds the '\r\n'
                strcat(&(*headerRet)[0], "\r\n");
            }

            // Goes to the next token
            tok = strtok(nullptr, "\r");
            // Increments i
            i++;
        }

        // Clears the memory
        free(rawC);
        return 0;
    }
}