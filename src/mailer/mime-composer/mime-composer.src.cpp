/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "mime-composer.src.hpp"

namespace Fannst::FSMTPServer::Mailer::Composer
{
    /**
    * Composes an MIME message, with proper encoding
    * @param options
    * @param comRet
    * @param comRetLen
    * @return
    */
    BYTE sexyComposingAlgorithm(const MailerComposerOptions &options, char **comRet, std::size_t *comRetLen)
    {
        std::vector<Types::MimeHeader> defaultHeaders{};
        std::vector<Types::MimeBodySection> bodySections{};

        char *headers = nullptr;
        std::size_t headersLen;

        char *body = nullptr;
        std::size_t bodyLen;

        char *messageBoundary = nullptr;
        std::size_t messageBoundaryLen;

        char *messageId = nullptr;
        std::size_t messageIdLen;

        BYTE rc = 0;

        // ----
        // Generates the boundary and message id
        // ----

        // Generates the message id
        _generateMessageID(&messageId, &messageIdLen);

        // Generates the boundary
        _generateBoundary(&messageBoundary, &messageBoundaryLen);

        std::cout << messageBoundary << std::endl;

        // ----
        // Inserts the default headers
        // ----

        // The X-Mailer
        defaultHeaders.emplace_back(Types::MimeHeader{"X-Mailer", GE_VERSION, false});
        // The credits for myself haha
        defaultHeaders.emplace_back(Types::MimeHeader{"X-Author", "Luke A.C.A. Rieff", false});

        // ----
        // The content type header
        // ----

        {
            // The default value for the content type header
            const char *contentTypeDefault = "multipart/alternative; boundary=\"";
            const char *contentTypeKeyDefault = "Content-Type";

            // Prepares the small buffer
            char *contentTypeValue = reinterpret_cast<char *>(malloc(
                    ALLOCATE_NULL_TERMINATION(messageBoundaryLen + strlen(&contentTypeDefault[0]) + 1)));
            PREP_ALLOCATE_INVALID(contentTypeValue);

            // Sets the null termination char
            contentTypeValue[0] = '\0';

            // Appends the default text
            strcat(&contentTypeValue[0], &contentTypeDefault[0]);

            // Appends the boundary
            strcat(&contentTypeValue[0], &messageBoundary[0]);

            // Appends the closing '"'
            strcat(&contentTypeValue[0], "\"");

            // Allocates memory for the key, and stores an string inside of it
            char *contentTypeKeyValue = reinterpret_cast<char *>(malloc(
                    ALLOCATE_NULL_TERMINATION(strlen(&contentTypeKeyDefault[0]))));
            PREP_ALLOCATE_INVALID(contentTypeKeyValue);

            // Copies the memory
            memcpy(&contentTypeKeyValue[0], &contentTypeKeyDefault[0], strlen(&contentTypeKeyDefault[0]) + 1);

            // The message content type, multipart/alternative in our case
            defaultHeaders.emplace_back(Types::MimeHeader{contentTypeKeyValue, contentTypeValue, true});
        }

        // ----
        // Allocates the start size, for the ret value
        // ----

        // Sets the length of the return value
        *comRetLen = 1;

        // Allocates one byte
        *comRet = reinterpret_cast<char *>(malloc(1));
        PREP_ALLOCATE_INVALID(*comRet)

        // Sets the null termination char
        (*comRet)[0] = '\0';

        // ----
        // Prepares the content sections
        // ----

        if (options.o_UseExistingSections)
        { // Use existing body sections

            // Stores copies of the existing body sections inside of the body sections we use
            for (const Types::MimeBodySection &section : options.o_ExistingSections)
                bodySections.push_back(section);
        } else if (options.o_HTML != nullptr && options.o_PlainText != nullptr)
        { // Use both HTML and Plain Text

            // Stores the HTML body section
            bodySections.emplace_back(Types::MimeBodySection{
                0,
                options.o_HTML,
                {},
                Types::MimeContentType::TEXT_HTML
            });

            // Stores the plain text body section
            bodySections.emplace_back(Types::MimeBodySection{
                1,
                options.o_PlainText,
                {},
                Types::MimeContentType::TEXT_PLAIN
            });
        } else if (options.o_HTML != nullptr && options.o_PlainText == nullptr)
        { // Turn the HTML into Plain Text, and send it
            // TODO: Create HTML to plain text parser
        } else if (options.o_PlainText != nullptr && options.o_HTML == nullptr)
        { // Generate simple html from
            // TODO: Create plain text to HTML parser
        } else
        { // Return error
            PREP_ERROR_SIN("Both the HTML, and PlainText are left empty, there are also nu existing sections.")

            // Sets the return code
            rc = -1;
        }

        // ----
        // Starts generating the headers, if no error
        // ----

        if (rc == 0)
        {
            // Generates the headerss
            generateHeaders(defaultHeaders, &headers, &headersLen);

            // ----
            // Adds the headers to the final message
            // ----

            // Sets the new final message size
            *comRetLen += headersLen;

            // Resizes the headers
            *comRet = reinterpret_cast<char *>(realloc(&(*comRet)[0], *comRetLen));
            PREP_ALLOCATE_INVALID(*comRet);

            // Concats the value
            strcat(&(*comRet)[0], &headers[0]);
        }

        // ----
        // Frees the memory
        // ----

        // Frees the variables
        free(messageBoundary);
        free(messageId);
        free(headers);
        free(body);

        // Frees the headers vector
        for (Types::MimeHeader &h : defaultHeaders)
        {
            if (!h.h_IsHeap) continue;
            free(const_cast<char *>(h.h_Key));
            free(const_cast<char *>(h.h_Value));
        }

        // If error, free result automatically
        if (rc != 0)
        {
            *comRetLen = 0;
            free(*comRet);
        }

        // Removes 1 from the len, to remove the null termination char
        *comRetLen--;

        return rc;
    }

    /**
     * Generates headers from an vector of headers
     * @param headers
     * @param hRet
     * @param hRetLen
     */
    void generateHeaders(std::vector<Types::MimeHeader> &headers, char **hRet, std::size_t *hRetLen)
    {
        // ----
        // Prepares the return value
        // ----

        // Sets the hRetLen
        *hRetLen = 1;

        // Allocates the required memory
        *hRet = reinterpret_cast<char *>(malloc(1));
        PREP_ALLOCATE_INVALID(*hRet);

        // Sets the null termination char
        (*hRet)[0] = '\0';

        // ----
        // Starts looping over the headers
        // ----

        // Starts the loop
        std::size_t kvPairSize;
        for (Types::MimeHeader &h : headers)
        {
            char *kvPair = nullptr;

            // ----
            // Creates the key - value pair
            // ----

            // Gets the size for the final string, of size key + 2 (":<WS>") + value + 2 ("<CR><LF>")
            kvPairSize = strlen(&h.h_Key[0]) + strlen(&h.h_Value[0]) + 4;

            // Allocates the required memory
            kvPair = reinterpret_cast<char *>(malloc(
                    ALLOCATE_NULL_TERMINATION(kvPairSize)
                    ));
            PREP_ALLOCATE_INVALID(kvPair);

            // Sets the null termination char
            kvPair[0] = '\0';

            // Copies the key into it
            strcat(&kvPair[0], &h.h_Key[0]);

            // Copies the ":<WS>" into it
            strcat(&kvPair[0], ": ");

            // Copies the value into it
            strcat(&kvPair[0], &h.h_Value[0]);

            // Appends the <CR><LF>
            strcat(&kvPair[0], "\r\n");

            // ----
            // Appends it to the final result
            // ----

            // Adds the current kvPairSize to the result size
            *hRetLen += kvPairSize;

            // Reallocates the result memory
            *hRet = reinterpret_cast<char *>(realloc(&(*hRet)[0], *hRetLen));
            PREP_ALLOCATE_INVALID(*hRet);

            // Concats the current kvPair
            strcat(&(*hRet)[0], &kvPair[0]);

            // ----
            // Frees the memory
            // ----

            free(kvPair);
        }

        // Removes one from the hRetLen, to get the string len without null termination char
        *hRetLen--;
    }
}