/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <vector>

#include "../pre.hpp"

namespace Fannst::FSMTPServer::Types
{
    typedef enum
    {
        INVALID = 0,
        TEXT_HTML,
        TEXT_PLAIN,
        MULTIPART_ALTERNATIVE,
        MULTIPART_MIXED
    } MimeContentType;

    typedef struct
    {
        char *h_Key;
        char *h_Value;
    } MimeHeader;

    typedef struct
    {
        BYTE s_Index;
        char *s_Content;
        std::vector<MimeHeader> s_FullHeaders;
    } MimeMultipartBodySection;
}