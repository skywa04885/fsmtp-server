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
        const char *h_Key;
        const char *h_Value;
    } MimeHeader;

    typedef struct
    {
        BYTE s_Index;
        char *s_Content;
        std::vector<MimeHeader> s_FullHeaders;
    } MimeMultipartBodySection;

    typedef struct
    {
        const char *e_Name;
        const char *e_Address;
    } EmailAddress;
}