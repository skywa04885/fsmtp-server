#pragma once

#include <vector>
#include <iostream>
#include <string>

namespace models
{
    typedef enum {
        EMAIL_CONTENT_HTML,
        EMAIL_CONTENT_PLAIN_TEXT
    } EmailContentType;

    typedef struct {
        std::string e_Content;
        EmailContentType e_Type;
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
        // The dates
        uint64_t m_Timestamp;
        uint64_t m_ReceiveTimestamp;
        // The full data vectors
        std::vector<EmailAddress> m_From;
        std::vector<EmailAddress> m_To;
        std::vector<EmailHeader> m_FullHeaders;
        std::vector<EmailContentSection> m_Content;
    };

    namespace parsers
    {
        void normalizeWhitespace(std::string& target);

        int parseAddress(const std::string& raw, EmailAddress& target);
        int parseAddressList(const std::string& raw, std::vector<EmailAddress>& target);
        int parseHeaders(const std::string& raw, std::vector<EmailHeader>& target);
    };
};