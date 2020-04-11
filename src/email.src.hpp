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
#include <math.h>

#include <cassandra.h>

#define EMAIL_PARSE_DEBUG

#ifdef EMAIL_PARSE_DEBUG
#define PRINT_MIME_PARSER_DEBUG(a) std::cout << "Debug: \033[32m[MIME Parser 1.0]\033[0m: " << a << std::endl
#else
#define PRINT_MIME_PARSER_DEBUG(a)
#endif

namespace models
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
        std::string e_Key;
        std::string e_Value;
    } EmailHeader;

    typedef struct {
        std::string e_Content;
        EmailContentSectionType e_Type;
        std::vector<EmailHeader> e_FullHeaders;
        int e_Index;
    } EmailContentSection;

    typedef struct {
        std::string e_Name;
        std::string e_Address;
    } EmailAddress;

    class Email
    {
    public:
        // The quick access variables
        EmailAddress m_TransportTo;
        EmailAddress m_TransportFrom;
        std::string m_Subject;
        std::string m_MessageID;
        std::string m_Date;
        std::string m_Boundary;
        EmailContentType m_ContentType;
        long m_Bucket;
        // User data
        CassUuid m_UUID;
        CassUuid m_UserUUID;
        // The dates
        long m_Timestamp;
        long m_ReceiveTimestamp;
        // The full data vectors
        std::vector<EmailAddress> m_From;
        std::vector<EmailAddress> m_To;
        std::vector<EmailHeader> m_FullHeaders;
        std::vector<EmailContentSection> m_Content;
        // Methods
        int save(CassSession *session);
        static long getCurrentBucket();
    };
};

// Overloads the email content type enum
inline std::ostream &operator << (std::ostream &out, models::EmailContentType const &data)
{
    switch (data)
    {
        case models::EmailContentType::EMAIL_CT_MULTIPART_ALTERNATIVE: {
            out << "Multipart Alternative ( Multiple sections, with different types )";
            break;
        }
        case models::EmailContentType::EMAIL_CT_TEXT_PLAIN: {
            out << "Plain Text ( Single text only / markup language section )";
            break;
        }
    }
    return out;
}

// Overloads the email section type enum
inline std::ostream &operator << (std::ostream &out, models::EmailContentSectionType const &data)
{
    switch (data)
    {
        case models::EmailContentSectionType::EMAIL_CS_HTML: {
            out << "HTML ( Hyper Text Markup Language )";
            break;
        }
        case models::EmailContentSectionType::EMAIL_CS_TEXT_PLAIN: {
            out << "Plain Text";
            break;
        }
    }
    return out;
}

// Overloads the EmailAddress struct
inline std::ostream &operator << (std::ostream &out, models::EmailAddress const &data)
{
    out << (data.e_Name.empty() ? "Unknown" : data.e_Name) << " <" << data.e_Address << ">";
    return out;
}

// Overloads the EmailAddress struct
inline std::ostream &operator << (std::ostream &out, models::EmailHeader const &data)
{
    out << data.e_Key << ": " << data.e_Value;
    return out;
}

// operator overloads
inline std::ostream &operator << (std::ostream &out, models::Email const &data)
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
        out << '\t' << "  - \033[34m[Section Type]\033[0m: \033[33m" << section.e_Type << "\033[0m" << std::endl;
        out << '\t' << "  - \033[34m[Section Index]\033[0m: \033[33m" << section.e_Index << "\033[0m" << std::endl;
        out << '\t' << "  - \033[34m[Section Value]\033[0m: \033[33m" << section.e_Content << "\033[0m" << std::endl;
        out << '\t' << "  - \033[34m[Section Headers]\033[0m: \033[0m" << std::endl;
        // Loops over the headers
        std::size_t j = 0;
        for (auto& header : section.e_FullHeaders)
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
