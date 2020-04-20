/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <random>
#include <malloc.h>

#include "../../pre.hpp"
#include "../../types/mime.src.hpp"
#include "generators.src.hpp"

namespace Fannst::FSMTPServer::Mailer::Composer
{
    typedef struct
    {
        const char *o_KeySelector{nullptr};
        const char *o_Domain{nullptr};
        const char *o_PrivateKeyFile{nullptr};
        bool o_EnableDKIM{false};
    } MailerComposerDKIMOptions;

    typedef struct
    {
        MailerComposerDKIMOptions o_DKIM{};
        const char *o_Domain{nullptr};
        std::vector<Types::EmailAddress> o_From{};
        std::vector<Types::EmailAddress> o_To{};
        std::vector<Types::MimeHeader> o_CustomHeaders{};
        const char *o_Subject{nullptr};
        const char *o_HTML{nullptr};
        const char *o_PlainText{nullptr};
        bool o_UseExistingSections{false};
        std::vector<Types::MimeBodySection> o_ExistingSections{};
    } MailerComposerOptions;

    /**
     * Composes an MIME message, with proper encoding
     * @param options
     * @param comRet
     * @param comRetLen
     * @return
     */
    BYTE sexyComposingAlgorithm(const MailerComposerOptions &options, char **comRet, std::size_t *comRetLen);

    /**
     * Generates headers from an vector of headers
     * @param headers
     * @param hRet
     * @param hRetLen
     */
    void generateHeaders(std::vector<Types::MimeHeader> &headers, char **hRet, std::size_t *hRetLen);
}