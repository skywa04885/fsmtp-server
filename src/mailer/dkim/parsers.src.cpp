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
    int parseMimeMessage(const char *raw, char *headerRet, char *bodyRet)
    {
        // ----
        // Prepares the buffers for the results
        // ----

        // Prepares the headers return
        headerRet = reinterpret_cast<char *>(malloc(1));
        headerRet[0] = '\0';
        std::size_t headersBufferSize = 1;

        // Prepares the body return
        bodyRet = reinterpret_cast<char *>(malloc(1));
        bodyRet[0] = '\0';
        std::size_t bodyBufferSize = 1;

        // ----
        // Starts looping over the message, by tokenizing on "\r\n"
        // ----

        // Prepares the tokenizer
        char *rawC = const_cast<char *>(&raw[0]);
        char *tok = strtok(&rawC[0], "\r\n");

        // Creates the tokenizing loop
        while (tok != nullptr)
        {
            // Goes to the next token
            tok = strtok(nullptr, "\r\n");
        }
    }
}