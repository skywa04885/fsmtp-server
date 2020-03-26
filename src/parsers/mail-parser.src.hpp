/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <string>
#include <vector>
#include <tuple>

#include <cstring>

#include "../email.src.hpp"

/*
 * Namespace index:
 * 1. normWhitespace(std::string& target)
 * - Removes spaces in front, and back of an string, plus double spaces
 *
 * 2. rpfWhitespace(std::string& target)
 * - Removes the prefix and suffix whitespace
 *
 * 3. parseAddress(const std::string& raw, models::EmailAddress& target)
 * - Parses an single email address in MIME Format
 *
 * 4. parseAddressList(const std::string& raw, std::vector<models::EmailAddress>& target)
 * - Parses an list of addresses
 *
 * 5. parseMime(std::string& raw, models::Email& target)
 * - Parses an Mime message
 *
 * 6. parseHeaderArgumentsValue(std::string &raw, std::string &key, std::string &value)
 * - Parses the value and key of an header argument
 *
 * 7. parseHeaderArguments(std::string& raw, std::vector<std::string>& target)
 * - Parses the header arguments
 *
 * 8. int parseHeader(std::string &raw, std::vector<models::EmailHeader>& headers);
 * - Parses an header
 */

namespace parsers
{
    /*
     * 1. rpfWhitespace
     */

    void rpfWhitespace(std::string& target);

    /*
     * 2. normWhitespace
     */

    void normWhitespace(std::string& target);

    /*
     * 3. parseAddress
     */

    int parseAddress(const std::string& raw, models::EmailAddress& target);

    /*
     * 4. parseAddressList
     */

    int parseAddressList(const std::string& raw, std::vector<models::EmailAddress>& target);

    /*
     * 5. parseMime
     */

    int parseMime(std::string& raw, models::Email& target);

    /*
     * 6. parseHeaderArgumentsValue
     */

    int parseHeaderArgumentsValue(std::string &raw, std::string &key, std::string &value);

    /*
     * 7. parseHeaderArguments
     */

    int parseHeaderArguments(std::string& raw, std::vector<std::string>& target);

    /*
     * 8. parseHeader
     */

    int parseHeader(std::string &raw, std::vector<models::EmailHeader>& headers);
};