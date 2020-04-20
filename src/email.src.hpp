/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <vector>
#include <iostream>
#include <chrono>
#include <string>
#include <cstring>

#include <math.h>
#include <cassandra.h>

#include "types/mime.src.hpp"
#include "db/mime-helpers.src.hpp"

namespace Fannst::FSMTPServer::Models
{
    typedef enum {
        EMAIL_CT_MULTIPART_ALTERNATIVE,
        EMAIL_CT_TEXT_PLAIN,
        EMAIL_CT_TEXT_HTML
    } EmailContentType;
    
    typedef enum {
        EMAIL_CS_HTML,
        EMAIL_CS_TEXT_PLAIN
    } EmailContentSectionType;

    typedef struct {
        std::string e_Content;
        EmailContentSectionType e_Type;
        std::vector<Types::MimeHeader> e_FullHeaders;
        int e_Index;
    } EmailContentSection;

    class Email
    {
    public:
        // The quick access variables
        Types::EmailAddress m_TransportTo;
        Types::EmailAddress m_TransportFrom;
        const char *m_Subject{nullptr};
        const char *m_MessageID{nullptr};
        long m_Date;
        const char *m_Boundary{nullptr};
        Types::MimeContentType m_ContentType;
        long m_Bucket;
        // User data
        CassUuid m_UUID;
        CassUuid m_UserUUID;
        // The dates
        long m_Timestamp;
        long m_ReceiveTimestamp;
        // The full data vectors
        std::vector<Types::EmailAddress> m_From;
        std::vector<Types::EmailAddress> m_To;
        std::vector<Types::MimeHeader> m_FullHeaders;
        std::vector<Types::MimeBodySection> m_Content;
        // Methods

        /**
         * Saves an email inside cassandra
         * @param session
         * @return
         */
        int save(CassSession *session);

        /**
         * Gets the current email bucket
         */
        static long getCurrentBucket();

        /**
         * Gets an email by uuid
         * @param cassSession
         * @param uuid
         * @param target
         * @return
         */
        static BYTE getMessage(CassSession *cassSession, long bucket, CassUuid userUuid, CassUuid &uuid, Email &target);
        // Constructor and destructor
        ~Email()
        {
            // Frees the const char pointers
            free(const_cast<char *>(this->m_Subject));
            free(const_cast<char *>(this->m_Boundary));
            free(const_cast<char *>(this->m_MessageID));

            // Frees the headers
            for (auto &a : this->m_FullHeaders)
            {
                free(const_cast<char *>(a.h_Value));
                free(const_cast<char *>(a.h_Key));
            }

            // Frees the addresses
            free(const_cast<char *>(this->m_TransportFrom.e_Name));
            free(const_cast<char *>(this->m_TransportFrom.e_Address));
            free(const_cast<char *>(this->m_TransportTo.e_Address));
            free(const_cast<char *>(this->m_TransportTo.e_Name));

            // Frees the address lists
            for (auto &a : this->m_From)
            {
                free(const_cast<char *>(a.e_Address));
                free(const_cast<char *>(a.e_Name));
            }

            for (auto &a : this->m_To)
            {
                free(const_cast<char *>(a.e_Address));
                free(const_cast<char *>(a.e_Name));
            }

            // Frees the content sections
            for (auto &s : this->m_Content)
            {
                free(const_cast<char *>(s.s_Content));

                // Frees the content headers
                for (auto &h : s.s_FullHeaders)
                {
                    free(const_cast<char *>(h.h_Value));
                    free(const_cast<char *>(h.h_Key));
                }
            }
        }
    };
};

// Overloads the email content type enum
inline std::ostream &operator << (std::ostream &out, Fannst::FSMTPServer::Models::EmailContentType const &data)
{
    switch (data)
    {
        case Fannst::FSMTPServer::Models::EmailContentType::EMAIL_CT_MULTIPART_ALTERNATIVE: {
            out << "Multipart Alternative ( Multiple sections, with different types )";
            break;
        }
        case Fannst::FSMTPServer::Models::EmailContentType::EMAIL_CT_TEXT_PLAIN: {
            out << "Plain Text ( Single text only / markup language section )";
            break;
        }
    }
    return out;
}

// Overloads the email section type enum
inline std::ostream &operator << (std::ostream &out, Fannst::FSMTPServer::Models::EmailContentSectionType const &data)
{
    switch (data)
    {
        case Fannst::FSMTPServer::Models::EmailContentSectionType::EMAIL_CS_HTML: {
            out << "HTML ( Hyper Text Markup Language )";
            break;
        }
        case Fannst::FSMTPServer::Models::EmailContentSectionType::EMAIL_CS_TEXT_PLAIN: {
            out << "Plain Text";
            break;
        }
    }
    return out;
}

// Overloads the EmailAddress struct
inline std::ostream &operator << (std::ostream &out, Fannst::FSMTPServer::Types::EmailAddress const &data)
{
    out << data.e_Name << " <" << data.e_Address << ">";
    return out;
}

// Overloads the EmailAddress struct
inline std::ostream &operator << (std::ostream &out, Fannst::FSMTPServer::Types::MimeHeader const &data)
{
    out << data.h_Key << ": " << data.h_Value;
    return out;
}

// operator overloads
inline std::ostream &operator << (std::ostream &out, Fannst::FSMTPServer::Models::Email const &data)
{
    // Appends the title
    out << "\033[34m[Email Instance]\033[0m:" << std::endl;
    // Appends the variables
    out << " - \033[34m[Transport To]\033[0m: \033[33m" << data.m_TransportTo << "\033[0m" << std::endl;
    out << " - \033[34m[Transport From]\033[0m: \033[33m" << data.m_TransportFrom << "\033[0m" << std::endl;
    out << " - \033[34m[Content Type]\033[0m: \033[33m" << data.m_ContentType << "\033[0m" << std::endl;
    out << " - \033[34m[Subject]\033[0m: \033[33m" << data.m_Subject << "\033[0m" << std::endl;
    out << " - \033[34m[Message ID\033[0m: \033[33m" << data.m_MessageID << "\033[0m" << std::endl;
    out << " - \033[34m[Message Date]\033[0m: \033[33m" << data.m_Date << "\033[0m" << std::endl;
    out << " - \033[34m[Boundary]\033[0m: \033[33m" << data.m_Boundary << "\033[0m" << std::endl;

    char output[64];
    cass_uuid_string(data.m_UserUUID, reinterpret_cast<char *>(&output));
    out << " - \033[34m[User UUID]\033[0m: \033[33m" << output << "\033[0m" << std::endl;
    cass_uuid_string(data.m_UUID, reinterpret_cast<char *>(&output));
    out << " - \033[34m[Message UUID]\033[0m: \033[33m" << output << "\033[0m" << std::endl;

    // Appends the mail to
    out << " - \033[34m[Mail To]\033[0m: " << std::endl;
    // Loops over the addresses
    std::size_t i = 1;
    for (auto &address : data.m_To)
    {
        // Appends to the stream
        out << '\t' << i << ". \033[36m" << address << "\033[0m" << std::endl;
        // Increments i
        i++;
    }
    
    // Appends the mail from
    out << " - \033[34m[Mail From]\033[0m: " << std::endl;
    // Loops over the addresses
    i = 1;
    for (auto &address : data.m_From)
    {
        // Appends to the stream
        out << '\t' << i << ". \033[36m" << address << "\033[0m" << std::endl;
        // Increments i
        i++;
    }

    // Appends the content
    out << " - \033[34m[Content]\033[0m: " << std::endl;
    // Loops over the addresses
    i = 1;
    for (auto &section : data.m_Content)
    {
        // Appends to the stream
        out << '\t' << i << ". \033[34m[Content Section]\033[0m:" << std::endl;
        out << '\t' << "  - \033[34m[Section Type]\033[0m: \033[33m" << section.s_ContentType << "\033[0m" << std::endl;
        out << '\t' << "  - \033[34m[Section Index]\033[0m: \033[33m" << section.s_Index << "\033[0m" << std::endl;
        out << '\t' << "  - \033[34m[Section Value]\033[0m: \033[33m" << section.s_Content << "\033[0m" << std::endl;
        out << '\t' << "  - \033[34m[Section Headers]\033[0m: \033[0m" << std::endl;
        // Loops over the headers
        std::size_t j = 0;
        for (auto& header : section.s_FullHeaders)
        {
            // Prints the current header
            std::cout << "\t\t " << j << ". \033[33m" << header << "\033[0m" << std::endl;
            // Increments J
            j++;
        }
        // Increments i
        i++;
    }

    // Appends the content
    out << " - \033[34m[Headers]\033[0m: " << std::endl;
    // Loops over the headers
    std::size_t j = 0;
    for (auto& header : data.m_FullHeaders)
    {
        // Prints the current header
        std::cout << "\t " << j << ". \033[33m" << header << "\033[0m" << std::endl;
         // Increments J
        j++;
    }

    return out;
}
