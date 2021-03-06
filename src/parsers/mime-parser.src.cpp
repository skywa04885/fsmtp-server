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
                memmove(&tok[0], &tok[j], strlen(&tok[0]) - j + 1);

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
                *headRet = reinterpret_cast<char *>(realloc(*headRet, headRetBuffSize));

                // Appends the current token and the <CR><LF>
                if (!containsIndention) strcat(&(*headRet)[0], CRLF);
                strcat(&(*headRet)[0], &tok[0]);
            } else
            { // Append to body

                // Allocates the new size in the body buffer
                bodyRetBuffSize += strlen(&tok[0]) + 2;
                *bodyRet = reinterpret_cast<char *>(realloc(*bodyRet, bodyRetBuffSize));

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
            memmove(&(*bodyRet)[0], &(*bodyRet)[2], strlen(&(*bodyRet)[0]) - 1);

        if ((*headRet)[0] == '\r' && (*headRet)[1] == '\n')
            memmove(&(*headRet)[0], &(*headRet)[2], strlen(&(*headRet)[0]) - 1);

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
            headers.emplace_back(Types::MimeHeader{key, value, true});

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
        memcpy(&(*valRet)[0], &raw[i + 1], hLen - i);

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
        if (raw[0] == 'm' && strcmp(&raw[0], "multipart/alternative") == 0)
        { // is multipart/alternative
            return Types::MimeContentType::MULTIPART_ALTERNATIVE;
        } else if (raw[0] == 'm' && strcmp(&raw[0], "multipart/mixed") == 0)
        { // Is multipart/mixed
            return Types::MimeContentType::MULTIPART_MIXED;
        } else if (raw[0] == 't' && strcmp(&raw[0], "text/plain") == 0)
        { // Is multipart/mixed
            return Types::MimeContentType::TEXT_PLAIN;
        } else if (raw[0] == 't' && strcmp(&raw[0], "text/html") == 0)
        { // Is multipart/mixed
            return Types::MimeContentType::TEXT_HTML;
        } else return Types::MimeContentType::INVALID;
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
        (*address)[l-k-1] = '\0';

        // Copies the address into the reserved piece of memory
        memcpy(&(*address)[0], &raw[k+1], l-k-1);

        // ----
        // Parses and stores the name
        // ----

        {
            // Allocates the required memory
            t = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(k, 0)));
            t[k] = '\0';

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
            if((*name)[size] == ' ') (*name)[size] = '\0';
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
    /**
     * Parses an header value, which may contain both keyed parameters, and non-keyed parameters
     * @param raw
     * @param nkParams
     * @param kParams
     * @return
     */
    BYTE parseHeaderParameters(const char *raw, std::vector<const char *> &nkParams,
                               std::map<const char *, const char *> &kParams)
    {
        char *rawC = nullptr, *tok = nullptr, *t = nullptr, *tv = nullptr;

        std::size_t size, j, k, l;

        BYTE rc = 0;

        // ----
        // Creates an copy of the data
        // ----

        // Gets the length of raw
        size = strlen(&raw[0]);

        // Allocates the memory for the copy, and copies the memory data
        rawC = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(size, 0)));
        memcpy(&rawC[0], &raw[0], size + 1);

        // ----
        // Prepares the tokenizer
        // ----

        // Creates the tokenizer
        tok = strtok(&rawC[0], ";");

        // ----
        // Starts the loop
        // ----

        while (tok != nullptr)
        {
            // Gets the length of the token
            k = strlen(&tok[0]);

            // Checks if the token contains an '=', if so... We know it is most likely key value based
            j = 0;
            for (char *p = &tok[0]; *p != '\0' && *p != '='; p++) j++;

            // Checks if the equals sign was there, if not immediately append the result to the nkParams
            if (j == k)
            {
                // Allocates the memory, and stores an copy
                t = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(k, 0)));
                memcpy(&t[0], &tok[0], k + 1);

                // Pushes the data to an new pointer inside of the nkParams vector
                nkParams.emplace_back(t);
            } else
            {
                // ----
                // Gets the key
                // ----

                // Allocates the memory for the key, and stores an copy
                t = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(j, 0)));
                t[j] = '\0';
                memcpy(&t[0], &tok[0], j);

                // ----
                // Gets the value
                // ----

                // Gets the size to allocate
                l = k - j;

                // Allocates the memory for the value, and stores an copy
                tv = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(l, 0)));
                tv[l] = '\0';
                memcpy(&tv[0], &tok[j+1], l);

                // ----
                // If there, remove '"'
                // ----

                // If there is an '"' at the begin, remove it
                if (tv[0] == '"') memmove(&tv[0], &tv[1], strlen(&tv[0]));

                // Store J because, we use j 3 times, which can be reduced to one calculation with this
                j = strlen(&tv[0]);

                // If '"' at string end, remove it
                if (tv[j-1] == '"') tv[j-1] = '\0';

                // ----
                // Appends the to the result array
                // ----

                kParams.insert(std::make_pair(t, tv));
            }

            // Goes to the next token
            tok = strtok(nullptr, ";");
        }

        // ----
        // Frees the memory
        // ----

        free(rawC);

        return rc;
    }

    /**
     * Parses an list of email addresses
     * @param raw
     * @param ret
     * @return
     */
    BYTE parseAddressList(const char *raw, std::vector<Types::EmailAddress> &ret)
    {
        char *rawC = nullptr, *tok = nullptr, *name = nullptr, *address = nullptr;

        BYTE rc = 0;

        std::size_t tempLen;

        // ----
        // Creates an copy of the data
        // ----

        // Gets the length of raw
        tempLen = strlen(&raw[0]);

        // Allocates the required memory
        rawC = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(tempLen, 0)));

        // Copies the string
        memcpy(&rawC[0], &raw[0], tempLen + 1);

        // ----
        // Prepares the tokenizer
        // ----

        // Gets the first token
        tok = strtok(&rawC[0], ",");

        // ----
        // Starts the loop
        // ----

        while (tok != nullptr)
        {
            // Parses the address
            if (parseAddress(&tok[0], &name, &address) < 0)
            {
                // Sets the return code
                rc = -1;

                // Goes to the end
                goto parseAddressList;
            }

            // Pushes the data to the vector
            ret.emplace_back(Types::EmailAddress{name, address});

            // Goes to the next token
            tok = strtok(nullptr, ",");
        }

        // ----
        // The end
        // ----

    parseAddressList:

        // ----
        // Frees the memory
        // ----

        free(rawC);

        return rc;
    }

    /**
     * Parses an multipart/alternative body
     * @param raw
     * @param boundary
     * @param target
     * @return
     */
    BYTE parseMultipartAlternativeBody(const char *raw, const char *boundary,
                                       std::vector<Types::MimeBodySection> &target) {
        char *rawC = nullptr, *tok = nullptr, *boundaryNewSection = nullptr, *boundaryEnd = nullptr,
                *sectionBuffer = nullptr, *bodyRet = nullptr, *headerRet = nullptr, *tempFormattingBuf = nullptr;
        size_t tempSize, sectionBufferSize, tempFormattingBufSize;
        std::vector<char *> tempSections{};
        std::vector<Types::MimeHeader> tempHeaders{};
        Types::MimeContentTransferEncoding tempTransferEncoding;
        Types::MimeContentType tempContentType;
        bool isEnd, newSection;
        BYTE rc = 0;

        // ----
        // Prepares the boundary's
        // ----

        // Gets the length of the boundary
        tempSize = strlen(&boundary[0]);

        // Allocates the required memory
        boundaryNewSection = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(tempSize, 2)));
        boundaryEnd = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(tempSize, 4)));

        // Adds the null termination chars
        boundaryNewSection[0] = '\0';
        boundaryEnd[0] = '\0';

        // Creates the strings
        strcat(&boundaryNewSection[0], "--");
        strcat(&boundaryNewSection[0], &boundary[0]);

        strcat(&boundaryEnd[0], "--");
        strcat(&boundaryEnd[0], &boundary[0]);
        strcat(&boundaryEnd[0], "--");

        // ----
        // Creates an copy of the raw data
        // ----

        // Gets the size of the raw string
        tempSize = strlen(&raw[0]);

        // Allocates memory for the copy
        rawC = reinterpret_cast<char *>(malloc(ALLOCATE_NULL_TERMINATION(tempSize)));

        // Copies the memory
        memcpy(&rawC[0], &raw[0], tempSize + 1);

        // ----
        // Prepares the tokenizer
        // ----

        // Creates the tokenizer
        tok = strtok(&rawC[0], "\r");

        // ----
        // Starts looping
        // ----

        // Starts looping
        isEnd = false;
        newSection = false;
        while (tok != nullptr) {
            // Gets the length of the token
            tempSize = strlen(&tok[0]);

            // ----
            // Prepares the token
            // ----

            // Checks if there is an '\n' in the begin, if so remove it
            if (tok[0] == '\n') memmove(&tok[0], &tok[1], tempSize);

            // ----
            // Checks if it is an boundary, if so... The new section or end
            // ----

            if (tok[0] == '-' && tok[1] == '-') {
                // Checks if it is the end
                isEnd = (strcmp(&tok[0], &boundaryEnd[0]) == 0);
                newSection = (strcmp(&tok[0], &boundaryNewSection[0]) == 0);

                // Checks if it is an new section boundary, else if it is an end boundary
                if (newSection || isEnd) { // An new section starts

                    // ----
                    // Processes the result, if not zero.. Else it will do nothing
                    // ----

                    if (sectionBuffer != nullptr) {
                        // Pushes the results to the buffer
                        tempSections.emplace_back(sectionBuffer);
                    }

                    // ----
                    // Generates the new section buffer
                    // ----

                    // Allocates 1 byte of buffer
                    sectionBuffer = reinterpret_cast<char *>(malloc(1));

                    // Sets the buffer size to 1
                    sectionBufferSize = 1;

                    // Sets the first char of the buffer to '\0' to allow future string operations
                    sectionBuffer[0] = '\0';

                    // Goes to the next token
                    tok = strtok(nullptr, "\r");

                    continue;
                }
            }

            // ----
            // Appends the current string to the section buffer
            // ----

            // Adds the length of the current token to the section buffer size
            sectionBufferSize += strlen(&tok[0]) + 2;

            // Resizes the section buffer
            sectionBuffer = reinterpret_cast<char *>(realloc(&sectionBuffer[0], sectionBufferSize));
            if (!sectionBuffer)
            {
                std::cerr << "Could not reallocate memory while parsing message body in file: " << __FILE__ <<
                ", at line: " << __LINE__ << std::endl;
            }

            // Appends the current token to the section buffer
            strcat(&sectionBuffer[0], &tok[0]);

            // Appends the <CR><LF>
            strcat(&sectionBuffer[0], CRLF);

            // ----
            // Finishes
            // ----

            // Goes to the next token
            tok = strtok(nullptr, "\r");
        }

        // ----
        // processes the pieces
        // ----

        for (char *section : tempSections)
        {
            // ----
            // Splits the section body and headers
            // ----

            // Separates the headers and body
            separateHeadersAndBody(section, &headerRet, &bodyRet);

            // Parses the headers, and stores them
            parseHeaders(headerRet, tempHeaders);

            // Gets the content type from the headers, and transfer encoding ... We need both of them \
                            to determine if we need any further parsing, for example 7 bit decoding
            for (const Types::MimeHeader &h : tempHeaders) {
                // ----
                // Prepares the copy
                // ----

                // Gets the size
                tempFormattingBufSize = strlen(&h.h_Key[0]);

                // Allocates memory for the lower case copy
                tempFormattingBuf = reinterpret_cast<char *>(malloc(
                        ALLOCATE_NULL_TERMINATION(tempFormattingBufSize)));

                // Copies the string
                memcpy(&tempFormattingBuf[0], &h.h_Key[0], tempFormattingBufSize + 1);

                // ----
                // Turns the string into lower case
                // ----

                // Converts the string to lower case
                for (char *p = &tempFormattingBuf[0]; *p != '\0'; p++)
                    *p = static_cast<char>(tolower(*p));

                // ----
                // Checks if the parameter is useful to us
                // ----

                if (tempFormattingBuf[0] == 'c' &&
                    strcmp(&tempFormattingBuf[0], "content-type") == 0) { // Is the "Content-Type"
                    tempContentType = getContentType(h.h_Value);
                } else if (tempFormattingBuf[0] == 'c'
                           && strcmp(&tempFormattingBuf[0], "content-transfer-encoding") ==
                              0) { // Is the "Content-Transfer-Encoding"
                    tempTransferEncoding = getTransferEncoding(h.h_Value);
                }

                // ----
                // Checks if there are any invalid
                // ----

                if (tempContentType == Types::MimeContentType::INVALID ||
                    tempTransferEncoding == Types::MimeContentTransferEncoding::MCT_INVALID) {
                    // TODO: Handle error
                }

                // ----
                // Frees the memory
                // ----

                free(tempFormattingBuf);
            }

            // ----
            // Checks if the body itself needs further processing, for example hex decoding for certain
            // values
            // ----

            // TODO: Create further processing for decoding

            // ----
            // Pushes the result
            // ----

            target.emplace_back(Types::MimeBodySection{
                    static_cast<int>(target.size()),
                    bodyRet,
                    tempHeaders,
                    tempContentType
            });

            // ----
            // Frees the memory
            // ----

            free(headerRet);
            free(section);

            tempHeaders.clear();
        }

        // ----
        // Frees the memory
        // ----

        free(rawC);
        free(boundaryNewSection);
        free(boundaryEnd);

        return rc;
    }

    /**
     * Gets the transfer encoding from string
     * @param raw
     * @return
     */
    Types::MimeContentTransferEncoding  getTransferEncoding(const char *raw)
    {
        if (raw[0] == 'b' && strcmp(&raw[0], "base64") == 0)
        { // is base64
            return Types::MimeContentTransferEncoding::MCT_BASE64;
        } else if (raw[0] == 'q' && strcmp(&raw[0], "quoted-printable") == 0)
        { // is quoted printable
            return Types::MimeContentTransferEncoding::MCT_QUOTED_PRINTABLE;
        } else if (raw[0] == '7' && strcmp(&raw[0], "7bit") == 0)
        { // is 7bit
            return Types::MimeContentTransferEncoding::MCT_7_BIT;
        } else if (raw[0] == 'b' && strcmp(&raw[0], "binary") == 0)
        { // Is binary
            return Types::MimeContentTransferEncoding::MCT_BINARY;
        } else if (raw[0] == '8' && strcmp(&raw[0], "8bit") == 0)
        { // Is 8bit
            return Types::MimeContentTransferEncoding::MCT_8_BIT;
        } else return Types::MimeContentTransferEncoding::MCT_INVALID;
    }
}