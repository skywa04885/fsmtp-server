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
     * Removes unwanted whitespace
     * @param a
     * @param aLen
     */
    void cleanWhitespace(const char *a, std::size_t aLen, char **ret)
    {
        // ----
        // Prepares the memory copy
        // ----

        // Clears existing memory, if it was not empty
        if (*ret != nullptr) free(*ret);
        // Reserves the memory
        *ret = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(aLen, 0)));
        // Resets the len so we can use it as an counter
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
            (*ret)[aLen] = *a;

            // Increments the indexes
            aLen++;
            a++;
        }

        // Sets the zero termination char
        (*ret)[aLen] = '\0';
    }

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
        std::size_t i, j;
        std::size_t headRetBuffSize;
        std::size_t bodyRetBuffSize;

        bool containsIndention;

        // ----
        // Prepares the headRet, and bodyRet
        // ----

        // Allocates one byte for the headers
        headRetBuffSize = 1;
        *headRet = reinterpret_cast<char *>(malloc(1));
        (*headRet)[0] = '\0';

        // Allocates one byte for the body
        bodyRetBuffSize = 1;
        *bodyRet = reinterpret_cast<char *>(malloc(1));
        (*bodyRet)[0] = '\0';

        // ----
        // Creates an copy of the raw string
        // ----

        // Gets the length of the raw string
        rawLen = strlen(&raw[0]);
        // Allocates the memory for the copy
        rawC = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(rawLen, 0)));
        // Copies the string
        memcpy(&rawC[0], &raw[0], rawLen + 1);

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
            // ----
            // Prepares the token, and sets headers ended to true, if needed
            // ----

            // Checks if there is an '\n' in the start which needs to be removed
            if (tok[0] == '\n') memmove(&tok[0], &tok[1], strlen(&tok[0]));

            // Checks if the string is empty
            if (tok[0] == '\0')
            { // String is empty, set headers ended to true
                headersEnded = true;
            }

            // Checks if it contains an indention, if so we want to merge it with the previous line
            if (tok[0] == ' ' || tok[0] == '\t')
            {
                // Detects where the chars start
                j = 0;
                for (char *p = &tok[0]; *p != '\0' && (*p == '\t' || *p == ' '); p++) j++;

                // Removes the indentation
                memmove(&tok[0], &tok[j], strlen(&tok[0]) - j - 1);

                // Sets the contains indentation to true
                containsIndention = true;
            }

            // ----
            // Appends the string to the according result, headers or body
            // ----

            if (!headersEnded)
            { // Append to headers

                // Allocates the new size in the headers buffer
                headRetBuffSize += strlen(&tok[0]) + 2;
                *headRet = reinterpret_cast<char *>(realloc(&(*headRet)[0], headRetBuffSize));

                // Appends the current token and the <CR><LF>
                if (!containsIndention) strcat(&(*headRet)[0], CRLF);
                strcat(&(*headRet)[0], &tok[0]);
            } else
            { // Append to body

                // Allocates the new size in the body buffer
                bodyRetBuffSize += strlen(&tok[0]) + 2;
                *bodyRet = reinterpret_cast<char *>(realloc(&(*bodyRet)[0], bodyRetBuffSize));

                // Appends the current token and the <CR><LF>
                if (!containsIndention) strcat(&(*bodyRet)[0], CRLF);
                strcat(&(*bodyRet)[0], &tok[0]);
            }

            // Goes to the next token
            tok = strtok(nullptr, "\r");
            i++;
            containsIndention = false;
        }

        // ----
        // Checks if there is an <CR><LF> at the begin of the string
        // ----

        if ((*bodyRet)[0] == '\r' && (*bodyRet)[1] == '\n')
            memmove(&(*bodyRet)[0], &(*bodyRet)[2], strlen(&(*bodyRet)[0]) - 2);

        if ((*headRet)[0] == '\r' && (*headRet)[1] == '\n')
            memmove(&(*headRet)[0], &(*headRet)[2], strlen(&(*headRet)[0]) - 2);

        // ----
        // Frees the memory
        // ----

        free(rawC);

        return 0;
    }

    /**
     * Parses the headers into an vector
     * @param raw
     * @param headers
     * @return
     */
    BYTE parseHeaders(const char *raw, std::vector<Types::MimeHeader> &headers)
    {
        char *rawC = nullptr;
        char *tok = nullptr;

        std::size_t rawLen;

        BYTE rc = 0;

        // ----
        // Creates an copy of the raw string
        // ----

        // Gets the length of raw
        rawLen = strlen(&raw[0]);

        // Allocates memory
        rawC = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(strlen(&raw[0]), 0)));

        // Copies the value, with the null termination char
        memcpy(&rawC[0], &raw[0], rawLen + 1);

        // ----
        // Prepares the tokenizer
        // ----

        tok = strtok(&rawC[0], "\r");

        // ----
        // Starts tokenizing
        // ----

        while (tok != nullptr)
        {
            char *key = nullptr;
            char *value = nullptr;

            // ----
            // Prepares the token
            // ----

            // Checks if there is an '\n' in the start which needs to be removed
            if (tok[0] == '\n') memmove(&tok[0], &tok[1], strlen(&tok[0]));

            // ----
            // Parses the header, and appends to vector
            // ----

            if (splitHeader(&tok[0], &key, &value) < 0)
            {
                rc = -1;
                goto parseHeadersEnd;
            }

            // Stores the header inside of the vector
            headers.emplace_back(Types::MimeHeader{key, value});

            // Goes to the next token
            tok = strtok(nullptr, "\r");
        }

        // ----
        // The end
        // ----

    parseHeadersEnd:

        // ----
        // Frees the memory
        // ----

        free(rawC);

        return rc;
    }

    /**
     * Splits an header based on ':'
     * @param raw
     * @param keyRet
     * @param valRet
     * @return
     */
    BYTE splitHeader(const char *raw, char **keyRet, char **valRet)
    {
        std::size_t i;
        std::size_t hLen;

        // ----
        // Finds the index of ':'
        // ----

        i = 0;
        for (const char *p = &raw[0]; *p != '\0' && *p != ':'; p++) i++;

        // ----
        // Checks if there was an ':'
        // ----

        if (strlen(&raw[0]) == i) return -1;

        // ----
        // Allocates the required memory, and copies the data into it
        // ----

        // Gets the length of the full header
        hLen = strlen(&raw[0]);

        // Allocates and copies the value of the key
        *keyRet = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(i, 0)));
        (*keyRet)[i] = '\0';
        memcpy(&(*keyRet)[0], &raw[0], i);

        // Allocates and copies the value of the value
        *valRet = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(hLen - i - 1, 0)));
        (*valRet)[hLen - i - 1] = '\0';
        memcpy(&(*valRet)[0], &raw[i + 1], hLen - i - 1);

        // ----
        // Removes the space at the begin, if there
        // ----

        // Removes the space from the val
        if ((*valRet)[0] == ' ') memmove(&(*valRet)[0], &(*valRet)[1], strlen(&(*valRet)[0]));

        return 0;
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

    /**
     * Gets the content type based value of header
     * @param raw
     * @return
     */
    Types::MimeContentType getContentType(const char *raw)
    {
        char *t = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(64, 0)));
        Types::MimeContentType result = Types::MimeContentType::INVALID;

        // ----
        // Starts comparing
        // ----

        if (raw[0] == 'm')
        { // starts with m

            // Copies the string
            t[21] = '\0';
            memcpy(&t[0], &raw[0], 21);

            // Compares
            if (strcmp(&t[0], "multipart/alternative") == 0)
            {
                result = Types::MimeContentType::MULTIPART_ALTERNATIVE;
            }

            // Copies the string
            t[15] = '\0';
            memcpy(&t[0], &raw[0], 15);

            // Compares
            if (strcmp(&t[0], "multipart/mixed") == 0)
            {
                result = Types::MimeContentType::MULTIPART_MIXED;
            }
        } else if (raw[0] == 't')
        { // starts with t
            // Copies the string
            t[9] = '\0';
            memcpy(&t[0], &raw[0], 9);

            // Compares
            if (strcmp(&t[0], "text/html") == 0)
            {
                result = Types::MimeContentType::TEXT_HTML;
            } else if (strcmp(&t[0], "text/plain") == 0)
            {
                result = Types::MimeContentType::TEXT_PLAIN;
            }
        }

        // ----
        // Frees the memory
        // ----

        free(t);

        return result;
    }

    /**
     * Parses the name and address from an Mime Address
     * @param raw
     * @param name
     * @param address
     * @return
     */
    BYTE parseAddress(const char *raw, char **name, char **address)
    {
        const char *p = nullptr;
        char *t = nullptr;
        std:size_t j, k, l, size;
        BYTE rc = 0;

        // ----
        // Prepares
        // ----

        size = strlen(&raw[0]);

        // ----
        // Finds the @ symbol
        // ----

        // Finds the '@' symbol
        j = 0;
        for (p = &raw[0]; *p != '\0' && *p != '@'; p++) j++;

        // Checks if there was even an '@' symbol, if not return with code -1
        if (size == j)
        {
            rc = -1;
            goto parseAddressEnd;
        }

        // ----
        // Finds the first < tag
        // ----

        // Sets k to the size of j
        k = j;

        // Gets the '@' symbol memory address
        p = &raw[j];

        // Loops over the
        for (std::size_t i = j; i > 0; i--)
        {
            if (*p == '<') break;
            // Decreases k
            k--;
            // Goes back one char
            p--;
        }

        // ----
        // Finds the closing tag
        // ----

        // Sets l to the size of j
        l = j;

        // Loops until it finds the closing tag
        for (p = &raw[j]; *p != '\0' && *p != '>'; p++) l++;

        // Checks if there was an closing tag
        if (l == size)
        {
            rc = -3;
            goto parseAddressEnd;
        }

        // ----
        // Stores the address
        // ----

        // We do not need to use the alloc, macro because this calculation already includes the null termination char
        *address = reinterpret_cast<char *>(malloc(l - k));
        (*address)[l -k] = '\0';

        // Copies the address into the reserved piece of memory
        memcpy(&(*address)[0], &raw[k+1], l-k-1);

        // ----
        // Parses and stores the name
        // ----

        {
            // Allocates the required memory
            t = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(k, 0)));
            t[size - k] = '\0';

            // Copies the string into it
            memcpy(&t[0], &raw[0], k);

            // ----
            // Removes spaces from the name, which are not required
            // ----

            // Cleans the whitespace
            cleanWhitespace(&t[0], k, name);

            // If whitespace at begin or end, remove it
            if ((*name)[0] == ' ') memmove(&(*name)[0], &(*name)[1], strlen(&(*name)[0]));
            size = strlen(&(*name)[0]);
            if((*name)[size-1] == ' ') memmove(&(*name)[size-1], &(*name)[size], size);
        }

        // ----
        // The end
        // ----

    parseAddressEnd:

        // ----
        // Frees the memory
        // ----

        free(t);

        return rc;
    }
}