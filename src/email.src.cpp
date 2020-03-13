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
            for (char &c : target)
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

        // Handles a specific section of a Mime message
        int handleMimeSection(const std::string& section, Email& target)
        {
            std::cout << "Section: " << section << std::endl << std::endl;
            // Returns that parsing went correct
            return 0;
        }

        // Parses an full Mime Message
        int parseMime(std::string& raw, Email& target)
        {
            /*
             * Splits the document up intro lines
             */

            // Inserts a 10 char before the end
            raw.insert(raw.begin(), 10);
            // The lines
            std::vector<std::string> lines;
            // Starts splitting the message into lines
            std::size_t current, previous = 0;
            // Gets the first occurrence
            current = raw.find("\r\n");
            // Loops over all the occurrences
            for (;;)
            {
                // Checks if should break
                if (current == std::string::npos) break;
                // Appends the current string
                lines.push_back(raw.substr(previous + 1, current - previous - 1));
                // Sets the previous
                previous = current + 1;
                // Gets the next occurrence
                current = raw.find("\r\n", previous);
            }
            // Appends the final string
            lines.push_back(raw.substr(previous + 1, current - previous - 1));

            /*
             * Starts binding sections, that belong together
             */

            // The result
            std::vector<std::string> result;
            // The temp
            std::string temp;
            // The current line started
            bool lIsIndented; // If there is indention
            bool lAbcStarted; // If the normal string is started
            bool lLastIndented;// If the last string was indented
            // Loops over all the lines
            for (auto& line : lines)
            {
                // Resets the values
                lIsIndented = false;
                lAbcStarted = false;
                // Loops over the chars
                for (char& c : line)
                {
                    if (!lAbcStarted && (c == ' ' || c == '\t')) lIsIndented = true;
                    else lAbcStarted = true;
                }
                // Checks if it is indented,
                // if is indented remove the not required white
                // space
                // TODO: Fix error, see terminal moron
                if (lIsIndented)
                {
                    // Sets the last to indented
                    lLastIndented = true;
                    // Normalizes the string
                    normalizeWhitespace(line);
                    // Appends the string to the temp
                    temp.append(line);
                } else if (!temp.empty()) {
                    // Appends line to the result
                    result.push_back(temp);
                    // Clears the temp
                    temp.clear();
                } else {
                    // Appends line to the result
                    result.push_back(line);
                }
            }

            // Prints the lines
            for (auto& line : result)
            {
                std::cout << line << std::endl;
            }

            // Returns
            return 0;
        }
    };
};