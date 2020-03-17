#include <cstring>
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

        // Parses an full Mime Message
        int parseMime(std::string& raw, Email& target)
        {
            /*
             * Splits the document up intro lines
             */

            // Removes the \r from the document,
            // so we can parse using '\n'
            raw.erase(std::remove(raw.begin(), raw.end(), '\r'), raw.end());

            // The lines
            std::vector<std::string> lines;
            // Starts splitting the message into lines
            std::size_t current, previous = 0;
            // Gets the first occurrence
            current = raw.find('\n');
            // Loops over all the occurrences
            for (;;)
            {
                // Checks if should break
                if (current == std::string::npos) break;
                // Appends the current string
                lines.push_back(raw.substr(previous, current - previous));
                // Sets the previous
                previous = current + 1;
                // Gets the next occurrence
                current = raw.find('\n', previous);
            }
            // Appends the final string
            lines.push_back(raw.substr(previous, current - previous));

            /*
             * Starts binding sections, that belong together
             *
             * TODO: Understand the code
             */

            // The result
            std::vector<std::string> result;
            // The temp, last line will serve as origin,
            // if multiple lines have been indented
            std::string originLine;     // The start line, used for compare
            bool currentLineIndented;   // If the last line was indented
            // Loops over all the lines
            for (auto& line : lines)
            {
                // Resets the reset required values
                currentLineIndented = false;

                /*
                 * 1. Check if current line is indented or not
                 * 2. Checks if the next line is indented, if so add to origin line,
                 * - if not set the origin line.
                 * 3. If the last line was indented, and the current not, append origin
                 * - to result
                 */

                // Checks if the string is indented
                for (char& c : line)
                {
                    if (c == ' ' || c == '\t') currentLineIndented = true;
                    else {
                        break; // Character is not a space or tab, so should break
                    }
                }

                // Checks if the current line is indented,
                // if so append to the last line
                if (currentLineIndented)
                {
                    bool abc = false;
                    std::string temp;
                    // Removes the whitespace
                    for (char& c : line)
                    {
                        // Checks if the current item is whitespace or not
                        if ((c == ' ' || c == '\t') && !abc)
                        {
                            continue;
                        } else {
                            abc = true;
                            temp += c;
                        }
                    }
                    // Appends the current line to the line of origin
                    originLine.append(" ").append(temp);
                }

                // If the current line is not indented
                // push the origin
                if (!currentLineIndented) {
                    result.push_back(originLine);
                    // Sets the origin line to empty
                    originLine.clear();
                }

                // If the current line is not indented
                // turn it into the origin line,
                // because we do not know if the next one is indented
                if (!currentLineIndented) {
                    // Makes the origin line
                    originLine = line;
                }
            }

            // Removes the first line, because it is bullshit
            result.erase(result.begin());

            /*
             * Separates the headers from the message
             */

            // The temp headers vector
            std::vector<std::string> tempHeaders;
            std::vector<std::string> tempBody;
            bool headersEnded = false; // If the headers ended
            bool iws = false; // Is whitespace

            // Loops over all the lines
            for (auto &line : result)
            {
                // Resets is whitespace
                iws = false;
                // Runs if the headers have not been ended
                if (!headersEnded)
                {
                    // Checks if the line is empty,
                    // because the vector we want
                    // to split the header
                    if (line.empty()) {
                        headersEnded = true;
                        continue;
                    }
                    // If not iws append to headers
                    tempHeaders.push_back(line);
                } else { // Headers ended
                    // Removes the empty lines
                    // if (line.empty()) continue;
                    // Appends the content to the content vector
                    tempBody.push_back(line);
                }
            }

            /*
             * Starts parsing the headers
             */

            // Loops over all the lines
            for (auto &line : tempHeaders)
            {
                // Parses the current line
                if (parseHeader(line, target.m_FullHeaders) < 0) return -1;
            }

            // Checks if any of the headers
            // is usable
            for (EmailHeader &header : target.m_FullHeaders)
            {
                if (header.e_Key[0] == 'S')
                {
                    if (header.e_Key.compare("Subject") == 0)
                    {
                        // Sets the subject value
                        target.m_Subject = header.e_Value;
                    }
                } else if (header.e_Key[0] == 'F')
                {
                    if (header.e_Key.compare("From") == 0)
                    {
                        // Parses the from value,
                        // and the method itself sets it
                        parseAddressList(header.e_Value, target.m_From);
                    }
                } else if (header.e_Key[0] == 'T')
                {
                    if (header.e_Key.compare("To") == 0)
                    {
                        // Parses the to value,
                        // and the method itself sets it
                        parseAddressList(header.e_Value, target.m_To);
                    }
                } else if (header.e_Key[0] == 'C')
                {
                    if (header.e_Key.compare("Content-Type") == 0)
                    {
                        // Parses the content type
                        std::vector<std::string> arguments;
                        parseHeaderArguments(header.e_Value, arguments);

                        // Loops over the vector,
                        // and checks which option
                        // currently should me modified
                        for (auto &argument : arguments)
                        {
                            // Checks which letter the current
                            // argument starts with in order to
                            // boost the performance
                            if (argument[0] == 'm')
                            {
                                // Checks if the type is multipart/alternative
                                if (argument.compare("multipart/alternative") == 0)
                                {
                                    target.m_ContentType = EmailContentType::EMAIL_CT_MULTIPART_ALTERNATIVE;
                                }
                            } else if (argument[0] == 't')
                            {
                                if (argument.compare("text/plain") == 0)
                                {
                                    target.m_ContentType = EmailContentType::EMAIL_CT_TEXT_PLAIN;
                                } else if (argument.compare("text/html") == 0)
                                {
                                    target.m_ContentType = EmailContentType::EMAIL_CT_TEXT_HTML;
                                }
                            } else
                            { // Is none of the existing, check if it is a key value type
                                // Parses the sub argument
                                std::string key;
                                std::string value;
                                if (parseHeaderArgumentsValue(argument, key, value) < 0) return -1;
                                
                                // Checks if the key is usefull to us
                                if (key[0] == 'b')
                                { // Begins with b
                                    if (key.compare("boundary") == 0)
                                    { // Is the boundary
                                        target.m_Boundary.append(value.substr(1, value.length() - 2));
                                    }
                                } else if (key[0] == 'c')
                                { // Begins with c
                                    if (key.compare("charset") == 0)
                                    { // Is charset

                                    }
                                }
                            }
                        }
                    }
                } else if (header.e_Key[0] == 'M')
                {
                    if (header.e_Key.compare("Message-ID") == 0)
                    {
                        target.m_MessageID = header.e_Value;
                    }
                } else if (header.e_Key[0] == 'D')
                {
                    if (header.e_Key.compare("Date") == 0)
                    {
                        target.m_Date = header.e_Value;
                    }
                }
            }
            
            // Starts parsing the body, based on the specified type
            if (target.m_ContentType == EmailContentType::EMAIL_CT_MULTIPART_ALTERNATIVE)
            { // Multiple content sections
                /*
                 * 1. Split up into multiple sections inside vector, with
                 * -Content type and other stuff.
                 */

                // The comparable variables
                std::string sectionBoundaryComparable;
                std::string sectionEndBoundaryComparable;
                sectionBoundaryComparable.append("--");
                sectionBoundaryComparable.append(target.m_Boundary);
                sectionEndBoundaryComparable.append("--");
                sectionEndBoundaryComparable.append(target.m_Boundary);
                sectionEndBoundaryComparable.append("--");

                // The result vector
                std::vector<EmailContentSection> content;

                // The current from, and to
                std::size_t sectionFrom = 0;
                std::size_t sectionTo = 0;

                // Loops over the lines
                for (auto &line : tempBody)
                {
                    // Checks if it should be considered as boundary
                    if (line[0] == '-')
                    {
                        // If it is document end
                        bool isDocEnd = line.compare(sectionEndBoundaryComparable) == 0;

                        // Checks if an boundary is hit
                        if  (line.compare(sectionBoundaryComparable) == 0 || isDocEnd)
                        {
                            // If the headers ended
                            bool headersEnded = false;

                            // Loops over the lines in current section
                            for (std::size_t i = sectionFrom; i < sectionTo; i++)
                            {
                                // Checks if the headers ended
                                if (!headersEnded)
                                {
                                    // Checks if current line is empty
                                    if (tempBody.at(i).empty())
                                    {
                                        // Parses the headers

                                        // Sets the header end to true
                                        // - no header anymore
                                        headersEnded = true;
                                    }
                                }
                            }

                            // Set the old section from
                            // to the curren to, so we
                            // can continue where we left
                            sectionFrom = sectionTo;
                        }

                        // Checks if it should break
                        if (isDocEnd)
                        {
                            // Breaks from the section
                            break;
                        }
                    }

                    // Increments the section to
                    sectionTo++;
                }
            } else if (target.m_ContentType == EmailContentType::EMAIL_CT_TEXT_PLAIN)
            { // Content type plain text / markup language
                
            } else if (target.m_ContentType == EmailContentType::EMAIL_CT_TEXT_HTML)
            { // Basic html content
                
            }


            // Returns
            return 0;
        }

        int parseHeaderArgumentsValue(std::string &raw, std::string &key, std::string &value)
        {
            // Gets the position of the =
            std::size_t pos = raw.find('=');

            // Checks if the string includes an =
            if (pos == std::string::npos) return -1;

            // Splits the string based on the =
            key = raw.substr(0, pos);
            value = raw.substr(pos + 1, raw.length() - pos - 1);

            // Returns code 0
            return 0;   
        }
        
        int parseHeaderArguments(std::string& raw, std::vector<std::string>& target)
        {
            /*
             * 1. Splits the string
             * 2. Optimizes the target
             */

            // Gets the raw C string
            const char *rawStr = raw.c_str();
            // Gets the first token
            char *tok = strtok(const_cast<char *>(rawStr), ";");
            // The current string
            std::string temp;

            // Starts the loop
            for (;;)
            {
                // Checks if the token is a nullptr
                if (tok == nullptr) break;
                // Removes the token whitespace
                temp = tok;
                normalizeWhitespace(temp);
                // Pushes to the result
                target.push_back(temp);
                // Gets the next token
                tok = strtok(nullptr, ";");
            }

            // Returns code 0
            return 0;
        }

        int parseHeader(std::string &raw, std::vector<EmailHeader>& headers)
        {
            /*
             * 1. Check if header contains :
             * 2. Split the header up in two parts, using the :
             * 3. Normalize outer whitespace on both sides, check if first on does not contain any
             */

            // Gets the first :
            int pos = raw.find_first_of(':');

            // Checks if the header contains an :
            if (pos == std::string::npos) return -1;

            // Creates the header result
            EmailHeader header;

            // Splits the header
            header.e_Key = raw.substr(0, pos);
            header.e_Value = raw.substr(pos + 1, raw.length() - pos);

            // Removes unnecessary spaces
            normalizeWhitespace(header.e_Value);
            normalizeWhitespace(header.e_Key);

            // Pushes the header
            headers.push_back(header);

            // Returns with code 0
            return 0;
        }
    };
};
