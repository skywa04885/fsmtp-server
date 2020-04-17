/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <string>
#include <tuple>
#include <vector>

#include <cassandra.h>

namespace Fannst::FSMTPServer::Models
{
    class User
    {
    public:
        User(
            const char *u_FullName,
            const char *u_Password,
            const char *u_Domain,
            const char *u_Username,
            const CassUuid &u_uuid,
            const char *u_Description,
            const char *u_Picture,
            const cass_int64_t &u_BirthDate,
            const cass_int64_t &u_CreationDate,
            const cass_int64_t &u_Bucket
        );
        ~User();

        int save();

        const char *u_FullName;
        const char *u_Password;
        const char *u_Domain;
        const char *u_Username;
        CassUuid u_uuid;
        const char *u_Description;
        const char *u_Picture;
        cass_int64_t u_BirthDate;
        cass_int64_t u_CreationDate;
        cass_int64_t u_Bucket;
    };

    class UserQuickAccess
    {
    public:
        UserQuickAccess(
            const char *u_Domain,
            const char *u_Username,
            const CassUuid &u_Uuid,
            const cass_int64_t &u_Bucket,
            const char *u_Password
        );
        UserQuickAccess();

        ~UserQuickAccess();

        const char *u_Domain;
        const char *u_Username;
        CassUuid u_Uuid;
        cass_int64_t u_Bucket;
        const char *u_Password;

        static int selectByDomainAndUsername(CassSession *session, const char *domain, const char *username, UserQuickAccess &target);
    };
};