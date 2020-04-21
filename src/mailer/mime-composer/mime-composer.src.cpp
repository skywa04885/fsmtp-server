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

        char *messageIdValue = nullptr;
        char *contentTypeValue = nullptr;
        char *dateValue = nullptr;
        char *returnPathValue = nullptr;
        char *fromValue = nullptr;
        char *toValue = nullptr;

        char *headers = nullptr;
        std::size_t headersLen;

        char *body = nullptr;
        std::size_t bodyLen;

        char *messageBoundary = nullptr;
        std::size_t messageBoundaryLen;

        char *messageId = nullptr;
        std::size_t messageIdLen;

        std::size_t tempLen;

        BYTE rc = 0;

        // ----
        // Generates the boundary and message id
        // ----

        // Generates the message id
        _generateMessageID(&messageId, &messageIdLen);

        // Generates the boundary
        _generateBoundary(&messageBoundary, &messageBoundaryLen);

        // ----
        // Inserts the default headers
        // ----

        // The X-SMTPMailer
        defaultHeaders.emplace_back(Types::MimeHeader{"X-Mailer", GE_VERSION, false});
        // The MIME-Version
        defaultHeaders.emplace_back(Types::MimeHeader{"MIME-Version", "1.0", false});
        // The credits for myself haha
        defaultHeaders.emplace_back(Types::MimeHeader{"X-Author", "Luke A.C.A. Rieff", false});
        // The subject, disable heap so the user can clear it later himself
        defaultHeaders.emplace_back(Types::MimeHeader{"Subject", options.o_Subject, false});

        // ----
        // Generate the return path
        // ----

        // Allocates memory for the return path
        returnPathValue = reinterpret_cast<char *>(malloc(
                ALLOCATE_NULL_TERMINATION(strlen(&options.o_From.at(0).e_Address[0]) + 2)));
        PREP_ALLOCATE_INVALID(returnPathValue);

        // Sets the null termination char
        returnPathValue[0] = '\0';

        // Appends the first '<'
        strcat(&returnPathValue[0], "<");

        // Appends the address
        strcat(&returnPathValue[0], &options.o_From.at(0).e_Address[0]);

        // Appends the last '>'
        strcat(&returnPathValue[0], ">");

        // Adds the return path
        defaultHeaders.emplace_back(Types::MimeHeader{"Return-Path", returnPathValue, false});

        // ----
        // Generate the message id, with '<' + MessageID + '>'
        // ----

        // Allocates the space for the message id value
        messageIdValue = reinterpret_cast<char *>(malloc(ALLOCATE_NULL_TERMINATION(messageIdLen + 2)));
        PREP_ALLOCATE_INVALID(messageIdValue);

        // Sets the null termination char
        messageIdValue[0] = '\0';

        // Concats the first '<'
        strcat(&messageIdValue[0], "<");

        // Concats the message id
        strcat(&messageIdValue[0], &messageId[0]);

        // Concats the last '>'
        strcat(&messageIdValue[0], ">");

        // Pushes it to the headers
        defaultHeaders.emplace_back(Types::MimeHeader{"Message-ID", messageIdValue, false});

        // ----
        // The content type header
        // ----

        //   The default value for the content type header
        const char *contentTypeDefault = "multipart/alternative; boundary=\"";

        // Prepares the small buffer
        contentTypeValue = reinterpret_cast<char *>(malloc(
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

        // The message content type, multipart/alternative in our case
        defaultHeaders.emplace_back(Types::MimeHeader{"Content-Type", contentTypeValue, false});

        // ----
        // The date header
        // ----

        {
            struct tm *timeInfo{};
            time_t rawTime;

            // Allocates the bytes for the date
            dateValue = reinterpret_cast<char *>(malloc(64));
            PREP_ALLOCATE_INVALID(dateValue);

            // Gets the current time
            time(&rawTime);
            timeInfo = localtime(&rawTime);

            // Formats the time, and stores it in the buffer
            strftime(dateValue, 64, "%a, %d %h %Y %T %z", timeInfo);
        }

        // Adds the header to the defaultHeaders
        defaultHeaders.emplace_back(Types::MimeHeader{"Date", dateValue, false});

        // ----
        // The from and to headers
        // ----

        // The from
        generateAddressList(options.o_From, &fromValue, &tempLen);

        // The to
        generateAddressList(options.o_To, &toValue, &tempLen);

        // Adds to the headers
        defaultHeaders.emplace_back(Types::MimeHeader{"From", fromValue, false});
        defaultHeaders.emplace_back(Types::MimeHeader{"To", toValue, false});

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
                {
                    Types::MimeHeader{
                        "Content-Type",
                        "text/html",
                        false
                    },
                    Types::MimeHeader{
                            "Content-Transfer-Encoding",
                            "quoted-printable",
                            false
                    }
                },
                Types::MimeContentType::TEXT_HTML
            });

            // Stores the plain text body section
            bodySections.emplace_back(Types::MimeBodySection{
                1,
                options.o_PlainText,
                {
                    Types::MimeHeader{
                        "Content-Type",
                        "text/plain",
                        false
                    },
                    Types::MimeHeader{
                        "Content-Transfer-Encoding",
                        "8bit",
                        false
                    }
                },
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
            *comRetLen += headersLen + 2;

            // Resizes the buffer
            *comRet = reinterpret_cast<char *>(realloc(&(*comRet)[0], *comRetLen));
            PREP_ALLOCATE_INVALID(*comRet);

            // Concats the value
            strcat(&(*comRet)[0], &headers[0]);
            strcat(&(*comRet)[0], "\r\n");

            // ----
            // Generates the body
            // ----

            // Generates the body
            generateMultipartAlternativeBody(bodySections, messageBoundary, messageBoundaryLen, &body, &bodyLen);

            // Sets the new final message size
            *comRetLen += bodyLen;

            // Resizes the buffer
            *comRet = reinterpret_cast<char *>(realloc(&(*comRet)[0], *comRetLen));
            PREP_ALLOCATE_INVALID(*comRet);

            // Concats the value
            strcat(&(*comRet)[0], &body[0]);

            // ----
            // If requested, perform DKIM signature on message
            // ----

            if (options.o_DKIM.o_EnableDKIM)
            {
                char *sigRet = nullptr;

                // ----
                // Configures the DKIM header
                // ----

                // Gets the current date in seconds
                std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
                long nowInSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

                // Creates the config
                DKIM::DKIMHeaderConfig config{};
                config.d_Ago = DKIM::DKIMCanAlgorithms::DCA_RELAXED_RELAXED;
                config.d_Domain = options.o_Domain;
                config.d_ExpireDate = nowInSeconds + 6000;
                config.d_SignDate = nowInSeconds;
                config.d_KeyS = options.o_DKIM.o_KeySelector;
                config.d_PKey = options.o_DKIM.o_PrivateKeyFile;

                // ----
                // Creates the DKIM signature
                // ----

                // Signs
                DKIM::sign(&(*comRet)[0], &sigRet, &config);

                // Overwrites the old return value with the new sigret
                free(*comRet);
                *comRet = sigRet;
            }
        }

        // ----
        // Frees the memory
        // ----

        // Frees the variables
        free(messageBoundary);
        free(messageId);
        free(headers);
        free(body);
        free(messageIdValue);
        free(contentTypeValue);
        free(dateValue);
        free(returnPathValue);
        free(fromValue);
        free(toValue);

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
        (*comRetLen)--;

        return rc;
    }

    /**
     * Generates headers from an vector of headers
     * @param headers
     * @param hRet
     * @param hRetLen
     */
    void generateHeaders(const std::vector<Types::MimeHeader> &headers, char **hRet, std::size_t *hRetLen)
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
        for (const Types::MimeHeader &h : headers)
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
        (*hRetLen)--;
    }

    /**
     * Generates an MIME address list from email addresses
     * @param addresses
     * @param lRet
     * @param lRetLen
     */
    void generateAddressList(const std::vector<Types::EmailAddress> &addresses, char **lRet, std::size_t *lRetLen)
    {
        // ----
        // Allocates the memory for the result
        // ----

        // Sets the result memory size, of one byte
        *lRetLen = 1;

        // Allocates memory for the return value
        *lRet = reinterpret_cast<char *>(malloc(1));
        PREP_ALLOCATE_INVALID(*lRet);

        // Sets the null termination char
        (*lRet)[0] = '\0';

        // ----
        // Starts looping over the addresses, and appends them
        // ----

        std::size_t addressLen;
        for (const Types::EmailAddress &address : addresses)
        {
            // ----
            // Prepares, and calculates the buffer length
            // ----

            // Calculates the length for the address, which will be appended, the format will be \
            <NAME>+<WS>+<GT>+<ADDRESS>+<LT>+<COMMA>

            // Checks if we will append the name
            if (address.e_Name[0] == '\0')
            { // Without the name
                addressLen = strlen(&address.e_Address[0]) + 4;
            } else
            { // With the name
                addressLen = strlen(&address.e_Name[0]) + strlen(&address.e_Address[0]) + 5;
            }

            // ----
            // Resizes the buffer
            // ----

            // Adds the length of the address to the result length
            *lRetLen += addressLen;

            // Resizes the buffer
            *lRet = reinterpret_cast<char *>(realloc(&(*lRet)[0], *lRetLen));
            PREP_ALLOCATE_INVALID(*lRet);

            // ----
            // Adds the results
            // ----

            // If required, appends the name with an whitespace
            if (address.e_Name[0] != '\0')
            {
                // Appends the name
                strcat(&(*lRet)[0], &address.e_Name[0]);

                // Appends the whitespace
                strcat(&(*lRet)[0], " ");
            }

            // Adds the '<'
            strcat(&(*lRet)[0], "<");

            // Adds the address
            strcat(&(*lRet)[0], &address.e_Address[0]);

            // Adds the ">, "
            strcat(&(*lRet)[0], ">, ");
        }

        // Removes one from the len, to get the size without the null termination
        (*lRetLen)--;

        // Removes the last ", "
        (*lRet)[(*lRetLen)-2] = '\0';

        // Removes two from the length
        (*lRetLen) -= 2;
    }

    /**
     * Generates the multipart body for an email message
     * @param sections
     * @param boundary
     * @param bRet
     * @param bRetLen
     */
    void generateMultipartAlternativeBody(const std::vector<Types::MimeBodySection> &sections,
            const char *boundary, const std::size_t &boundaryLen,
            char **bRet, std::size_t *bRetLen)
    {
        // ----
        // Prepares the result
        // ----

        // Sets the size
        *bRetLen = 1;

        // Allocates one byte of memory
        *bRet = reinterpret_cast<char *>(malloc(1));
        PREP_ALLOCATE_INVALID(*bRet);

        // Sets the null termination char
        (*bRet)[0] = '\0';

        // ----
        // Starts looping
        // ----

        std::size_t tempSize;
        for (const Types::MimeBodySection &section : sections)
        {
            char *tempHeaders = nullptr;
            char *tempBody = nullptr;

            // ----
            // Creates the headers + the boundary, and appends them to the result
            // ----

            // Generates the headers
            generateHeaders(section.s_FullHeaders, &tempHeaders, &tempSize);

            // Sets the new result buffer size
            *bRetLen += tempSize + boundaryLen + 8;

            // Resizes the result buffer
            *bRet = reinterpret_cast<char *>(realloc(&(*bRet)[0], *bRetLen));
            PREP_ALLOCATE_INVALID(*bRet);

            // Appends the boundary
            strcat(&(*bRet)[0], "--");
            strcat(&(*bRet)[0], boundary);
            strcat(&(*bRet)[0], "\r\n");

            // Appends the headers
            strcat(&(*bRet)[0], &tempHeaders[0]);
            strcat(&(*bRet)[0], "\r\n");

            // ----
            // Creates the body, and appends them to the result
            // ----

            // Sets the new result buffer size
            *bRetLen += strlen(&section.s_Content[0]);

            // Resizes the result buffer
            *bRet = reinterpret_cast<char *>(realloc(&(*bRet)[0], *bRetLen));
            PREP_ALLOCATE_INVALID(*bRet);

            // Appends the body
            strcat(&(*bRet)[0], &section.s_Content[0]);
            strcat(&(*bRet)[0], "\r\n");

            // ----
            // Frees the memory
            // ----

            free(tempHeaders);
            free(tempBody);
        }

        // ----
        // Appends the final end boundary
        // ----

        // Sets the new result buffer size
        *bRetLen += boundaryLen + 6;

        // Resizes the buffer
        *bRet = reinterpret_cast<char *>(realloc(&(*bRet)[0], *bRetLen));
        PREP_ALLOCATE_INVALID(*bRet);

        // Appends the boundary
        strcat(&(*bRet)[0], "--");
        strcat(&(*bRet)[0], &boundary[0]);
        strcat(&(*bRet)[0], "--\r\n");

        // Removes one from the length, to remove the null termination char
        (*bRetLen)--;
    }
}