#include "email.src.hpp"

namespace models
{
    namespace parsers
    {
        void normalizeWhitespace(std::string& target)
        {
            std::string temp = "";
            // Removes double whitespaces
            bool lww = false;
            // Loops over all the chars
            for (char& c : target)
            {
                if (c == ' ') {
                    if (!lww) {
                        temp += c;
                    }
                    // Sets lww to true, last was white
                    lww = true;
                } else {
                    // Appends the char
                    temp += c;
                    // Sets lww to false, last was white false
                    lww = false;
                }
            }
            // Removes, the prefix whitespace, or suffix whitespace
            if (temp[0] == ' ') temp = temp.substr(1, temp.length() - 1);
            if (temp[temp.length() - 1] == ' ') temp = temp.substr(0, temp.length() - 1);
            // Sets the result
            target = temp;
        }

        int parseAddress(const std::string& raw, EmailAddress& target)
        {
            // The temp string
            std::string temp;
            // Parses the name if possible
            if (raw[0] != '<' && raw.find_first_of('<') != std::string::npos)
            {
                // Gets the section where the name is
                temp = raw.substr(0, raw.find_first_of('<'));
                // Normalizes the whitespace
                normalizeWhitespace(temp);
                // Sets the value
                target.e_Name = temp;
            }
            // Checks if it should parse an address
            if (!target.e_Name.empty() || raw.find('<') != std::string::npos) {
                // Gets the start index
                int bStart = raw.find_last_of('<');
                // Gets the end index
                int bEnd = raw.find_first_of('>');
                // Gets the string
                temp = raw.substr(bStart + 1, bEnd - bStart - 1);
                // Normalizes the address
                normalizeWhitespace(temp);
                // Checks if not empty
                if (temp.empty()) return -1;
                // Stores the value
                target.e_Address = temp;
            } else if (target.e_Name.empty()) { // Address only
                // Sets the temp
                temp = raw;
                // Normalizes the address
                normalizeWhitespace(temp);
                // Sets the address
                target.e_Address = temp;
            } else {
                return -1;
            }
            // Returns code 0
            return 0;
        }

        int parseAddressList(const std::string& raw, std::vector<EmailAddress>& target)
        {
            // The current address
            EmailAddress currentAddress;
            // The current pos, and previous pos
            std::size_t current, previous = 0;
            // Gets the first , occurrence
            current = raw.find(',');
            // Loops over all the occurrences
            for (;;)
            {
                // Checks if should break
                if (current == std::string::npos) break;
                // Parses the email address
                if (parseAddress(raw.substr(previous, current - previous), currentAddress) < 0)
                {
                    return -1;
                }
                // Appends the address to the result
                target.push_back(currentAddress);
                // Sets the previous
                previous = current + 1;
                // Gets the next one
                current = raw.find(',', previous);
            }
            // Parses the final email address
            if (parseAddress(raw.substr(previous, current - previous), currentAddress) < 0)
            {
                return -1;
            }
            // Appends the final address to the result
            target.push_back(currentAddress);
            // End
            return 0;
        }

        int parseHeaders(const std::string& raw, std::vector<EmailHeader>& target)
        {

        }
    };
};