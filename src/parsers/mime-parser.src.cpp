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
        bool headersEnded;

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
        // Prepares the looping
        // ----

        // Gets the first occurrence of '\r'
        tok = strtok(&rawC[0], "\r");
        // Sets headers ended to false
        headersEnded = false;

        // ----
        // Starts the looping
        // ----

        while (tok != nullptr)
        {
            // Checks if there is an '\n' in the start which needs to be removed
            if (tok[0] == '\n') memmove(&tok[0], &tok[1], strlen(&tok[0]));

            // Checks if the current line is empty
            std::cout << "'" << tok << "'" << std::endl;

            // Goes to the next token
            tok = strtok(nullptr, "\r");
        }
    }

    /**
    * Splits an email address up in the domain, and username
    * @param raw
    * @param username
    * @param domain
    * @return
    */
    int splitAddress(const char *raw, char **username, char **domain)
    {
        std::size_t rawCpyLen;
        char *rawCpy = nullptr;
        char *p = nullptr;
        int i;

        // ----
        // Checks if the memory needs to be freed
        // ----

        if (*username != nullptr)
        {
            free(*username);
            *username = nullptr;
        }

        if (*domain != nullptr)
        {
            free(*domain);
            *domain = nullptr;
        }

        // ----
        // Creates an copy of the raw address
        // ----

        // Gets the string length
        rawCpyLen = ALLOC_CAS_STRING(strlen(&raw[0]), 0);
        // Allocates the memory and stores the copy
        rawCpy = reinterpret_cast<char *>(malloc(rawCpyLen));
        memcpy(&rawCpy[0], &raw[0], rawCpyLen);

        // ----
        // Starts looping over the parts
        // ----

        // Finds the index of the '@' symbol
        i = 0;
        for (p = &rawCpy[0]; *p != '\0'; p++)
        {
            if (*p == '@') break;
            i++;
        }

        // ----
        // Frees the memory
        // ----

        free(rawCpy);

        // Checks if the address is valid
        if (rawCpyLen-1 == i) return -1;

        // ----
        // Splits the address
        // ----

        // Stores the username
        *username = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(i, 0)));
        (*username)[i] = '\0';
        memcpy(&(*username)[0], &raw[0], i);

        // Stores the domain
        *domain = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(rawCpyLen-i-1, 0)));
        (*domain)[rawCpyLen-i-1] = '\0';
        memcpy(&(*domain)[0], &raw[i+1], rawCpyLen-i-1);

        return 0;
    }
}