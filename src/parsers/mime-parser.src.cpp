/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "mime-parser.src.hpp"

namespace Fannst::FSMTPServer::MIMEParser
{
    /**
     * Separates the headers from the body in MIME message
     * @param raw
     * @param headRet
     * @param bodyRet
     * @return
     */
    BYTE separateHeadersAndBody(const char *raw, char **headRet, char **bodyRet)
    {
        char *tok = nullptr;
        char *rawC = nullptr;
        std::size_t rawLen;

        // ----
        // Creates an copy of the raw string
        // ----

        // Gets the length of the raw string
        rawLen = strlen(&raw[0]);
        // Allocates the memory for the copy
        rawC = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(rawLen, 0)));
        // Copies the string
        memcpy(&rawC[0], &raw[0], rawLen);

        // ----
        // Starts looping over the body
        // ----

        // Gets the first occurrence of '\r'
        tok = strtok(&rawC[0], "\r");
    }
}