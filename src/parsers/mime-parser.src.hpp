/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <cstring>
#include <malloc.h>
#include <vector>
#include <map>

#include "../pre.hpp"
#include "../email.src.hpp"
#include "../types/mime.src.hpp"

namespace Fannst::FSMTPServer::MIMEParser
{
    /**
     * Removes unwanted whitespace
     * @param a
     * @param aLen
     */
    void cleanWhitespace(const char *a, std::size_t aLen, char **ret);

    /**
     * Separates the headers from the body in MIME message
     * @param raw
     * @param headRet
     * @param bodyRet
     * @return
     */
    BYTE separateHeadersAndBody(const char *raw, char **headRet, char **bodyRet);

    /**
     * Parses the headers into an vector
     * @param raw
     * @param headers
     * @return
     */
    BYTE parseHeaders(const char *raw, std::vector<Types::MimeHeader> &headers);

    /**
     * Splits an header based on ':'
     * @param raw
     * @param keyRet
     * @param valRet
     * @return
     */
    BYTE splitHeader(const char *raw, char **keyRet, char **valRet);

    /**
     * Splits an email address up in the domain, and username
     * @param raw
     * @param username
     * @param domain
     * @return
     */
    int splitAddress(const char *raw, char **username, char**domain);

    /**
     * Gets the content type based value of header
     * @param raw
     * @return
     */
    Types::MimeContentType getContentType(const char *raw);

    /**
     * Parses the name and address from an Mime Address
     * @param raw
     * @param name
     * @param address
     * @return
     */
    BYTE parseAddress(const char *raw, char **name, char **address);

    /**
     * Parses an header value, which may contain both keyed parameters, and non-keyed parameters
     * @param raw
     * @param nkParams
     * @param kParams
     * @return
     */
    BYTE parseHeaderParameters(const char *raw, std::vector<const char *> &nkParams,
            std::map<const char *, const char *> &kParams);

    /**
     * Parses an list of email addresses
     * @param raw
     * @param ret
     * @return
     */
    BYTE parseAddressList(const char *raw, std::vector<Types::EmailAddress> &ret);
}