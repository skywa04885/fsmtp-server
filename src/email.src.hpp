#pragma once

#include <vector>
#include <iostream>
#include <string>

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
        std::string e_Content;
        EmailContentSectionType e_Type;
        int e_Index;
    } EmailContentSection;

    typedef struct {
        std::string e_Key;
        std::string e_Value;
    } EmailHeader;

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
        // The dates
        uint64_t m_Timestamp;
        uint64_t m_ReceiveTimestamp;
        // The full data vectors
        std::vector<EmailAddress> m_From;
        std::vector<EmailAddress> m_To;
        std::vector<EmailHeader> m_FullHeaders;
        std::vector<EmailContentSection> m_Content;
    };
    
    // Raw text parsers, for an email instance
    namespace parsers
    {
        void normalizeWhitespace(std::string& target);

        int parseAddress(const std::string& raw, EmailAddress& target);
        int parseAddressList(const std::string& raw, std::vector<EmailAddress>& target);
        int parseMime(std::string& raw, Email& target);
        int parseHeader(std::string &raw, std::vector<EmailHeader>& headers);
        int parseHeaderArguments(std::string& raw, std::vector<std::string>& target);
        int parseHeaderArgumentsValue(std::string &raw, std::string &key, std::string &value);
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

// Overloads the EmailAddress struct
inline std::ostream &operator << (std::ostream &out, models::EmailAddress const &data)
{
    out << (data.e_Name.empty() ? "Unknown" : data.e_Name) << " <" << data.e_Address << ">";
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
    
    return out;
}
