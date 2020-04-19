/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "parse-message.src.hpp"

namespace Fannst::FSMTPServer::Server
{
    /**
     * Parses an raw mime message, and stores it into an email
     * @param raw
     * @param target
     * @return
     */
    BYTE parseMessage(const char *raw, Models::Email &target)
    {
        BYTE rc = 0;

        Timer t("MimeParser");

        char *headers = nullptr;
        char *body = nullptr;

        // ----
        // Separates the headers from the body
        // ----

        Fannst::FSMTPServer::MIMEParser::separateHeadersAndBody(raw, &headers, &body);

        // ----
        // Parses the headers
        // ----

        // Parses
        if (Fannst::FSMTPServer::MIMEParser::parseHeaders(headers, target.m_FullHeaders) < 0)
        {
            std::cout << "Went wrong ..." << std::endl;
        }

        // Checks for the required data, such as content type etc
        char *klw = reinterpret_cast<char *>(malloc(65));
        char *cmpT = reinterpret_cast<char *>(malloc(65));

        std::size_t hValueLen;

        for (auto &h : target.m_FullHeaders)
        {
            hValueLen = strlen(&h.h_Value[0]);

            // ----
            // Turns the key into an lower case char
            // ----

            // Copies the string value
            strcpy(&klw[0], &h.h_Key[0]);

            // Turns it into lower case
            for (char *p = &klw[0]; *p != '\0'; p++)
            {
                *p = static_cast<char>(tolower(*p));
            }

            // ----
            // Checks if the header is useful to us
            // ----

            if (klw[0] == 'm')
            { // Starts with m, possible "message-id"

                // Copies the text
                cmpT[10] = '\0';
                memcpy(&cmpT[0], &klw[0], 10);

                // Compares the strings
                if (strcmp(&cmpT[0], "message-id") == 0)
                { // Is the message id

                    // Allocates the memory for the message id
                    target.m_MessageID = reinterpret_cast<const char *>(malloc(
                            ALLOC_CAS_STRING(hValueLen, 0)));

                    // Copies the value
                    memcpy(&const_cast<char *>(target.m_MessageID)[0], &h.h_Value[0], hValueLen + 1);
                }
            } else if (klw[0] == 's')
            { // starts with 's' possible "subject"

                // Copies the text
                cmpT[7] = '\0';
                memcpy(&cmpT[0], &klw[0], 7);

                // Compares the strings
                if (strcmp(&cmpT[0], "subject") == 0)
                {
                    // Allocates the memory for the message id
                    target.m_Subject = reinterpret_cast<const char *>(malloc(
                            ALLOC_CAS_STRING(hValueLen, 0)));

                    // Copies the value
                    memcpy(&const_cast<char *>(target.m_Subject)[0], &h.h_Value[0], hValueLen + 1);
                }
            } else if (klw[0] == 'c')
            { // starts with 'c', possible "content-type"
                // Copies the text
                cmpT[12] = '\0';
                memcpy(&cmpT[0], &klw[0], 12);

                // Compares the strings
                if (strcmp(&cmpT[0], "content-type") == 0)
                {
                    std::vector<const char *> nkValues{};
                    std::map<const char *, const char *> kValues{};

                    // ----
                    // Starts parsing
                    // ----

                    // Parses the arguments
                    MIMEParser::parseHeaderParameters(h.h_Value, nkValues, kValues);

                    // Gets the content type
                    target.m_ContentType = MIMEParser::getContentType(nkValues.at(0));

                    // Finds the boundary
                    for (auto it = kValues.begin(); it != kValues.end(); it++)
                    {
                        // If not starting with b, it is not boundary
                        if (it->first[0] != 'b') continue;

                        // Checks if it is the boundary
                        if (strcmp(&it->first[0], "boundary") == 0)
                        {
                            // Allocates the memory
                            target.m_Boundary = reinterpret_cast<char *>(
                                    malloc(ALLOC_CAS_STRING(strlen(&it->second[0]), 0)));

                            // Copies the boundary
                            memcpy(const_cast<char *>(&target.m_Boundary[0]),
                                   &it->second[0], strlen(&it->second[0]) + 1);
                        }
                    }

                    // ----
                    // Frees the memory
                    // ----

                    // Frees the no key values
                    for (const char *a : nkValues) free(const_cast<char *>(a));

                    // Frees the keyed values
                    for (auto it = kValues.begin(); it != kValues.end(); it++)
                    {
                        free(const_cast<char *>(it->first));
                        free(const_cast<char *>(it->second));
                    }
                }
            } else if (klw[0] == 'f')
            { // Starts with 'f', possible "from"

                // Copies the text
                cmpT[4] = '\0';
                memcpy(&cmpT[0], &klw[0], 4);

                // Compares the strings
                if (strcmp(&cmpT[0], "from") == 0)
                {
                    // Parses the addresses
                    if (MIMEParser::parseAddressList(&h.h_Value[0], target.m_From) < 0)
                    {
                        // TODO: Handle error
                    }
                }
            } else if (klw[0] == 't')
            { // Starts with 't', possible 'to'

                // Copies the text
                cmpT[2] = '\0';
                memcpy(&cmpT[0], &klw[0], 2);

                // Compares the strings
                if (strcmp(&cmpT[0], "to") == 0)
                {
                    // Parses the addresses
                    if (MIMEParser::parseAddressList(&h.h_Value[0], target.m_To) < 0)
                    {
                        // TODO: Handle error
                    }
                }
            }
        }

        // ----
        // Parses the message body with the required algorithm
        // ----

        switch (target.m_ContentType)
        {
            // Parsing "multipart/alternative" using boundary
            case Types::MimeContentType::MULTIPART_ALTERNATIVE:
            {
                MIMEParser::parseMultipartAlternativeBody(&body[0], target.m_Boundary, target.m_Content);
                break;
            }
            case Types::TEXT_HTML:
            {
                // TODO: Clean up the strlen stuff

                // Allocates memory for the copy
                char *copy = reinterpret_cast<char *>(malloc(
                        ALLOCATE_NULL_TERMINATION(strlen(&body[0]))));

                // Copies data
                memcpy(&copy[0], &body[0], strlen(&body[0]));

                // Inserts the body section
                target.m_Content.emplace_back(Types::MimeBodySection{
                        0,
                        copy,
                        {},
                        Types::MimeContentType::TEXT_HTML
                });
                break;
            }
            case Types::TEXT_PLAIN:
            {
                // TODO: Clean up the strlen stuff

                // Allocates memory for the copy
                char *copy = reinterpret_cast<char *>(malloc(
                        ALLOCATE_NULL_TERMINATION(strlen(&body[0]))));

                // Copies data
                memcpy(&copy[0], &body[0], strlen(&body[0]));

                // Inserts the body section
                target.m_Content.emplace_back(Types::MimeBodySection{
                        0,
                        copy,
                        {},
                        Types::MimeContentType::TEXT_PLAIN
                });
                break;
            }
        }

        // ----
        // Frees the memory
        // ----

        free(klw);
        free(cmpT);
        free(headers);
        free(body);

        return rc;
    }
}