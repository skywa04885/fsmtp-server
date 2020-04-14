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
            const std::string &u_FullName,
            const std::string &u_Password,
            const std::string &u_Domain,
            const std::string &u_Username,
            const CassUuid &u_uuid,
            const std::string &u_Description,
            const std::string &u_Picture,
            const cass_int64_t &u_BirthDate,
            const cass_int64_t &u_CreationDate,
            const cass_int64_t &u_Bucket
        );
        ~User();

        int save();

        std::string u_FullName;
        std::string u_Password;
        std::string u_Domain;
        std::string u_Username;
        CassUuid u_uuid;
        std::string u_Description;
        std::string u_Picture;
        cass_int64_t u_BirthDate;
        cass_int64_t u_CreationDate;
        cass_int64_t u_Bucket;
    };

    class UserQuickAccess
    {
    public:
        UserQuickAccess(
            const std::string &u_Domain,
            const std::string &u_Username,
            const CassUuid &u_Uuid,
            const cass_int64_t &u_Bucket,
            const std::string &u_Password
        );
        UserQuickAccess();

        ~UserQuickAccess();

        std::string u_Domain;
        std::string u_Username;
        CassUuid u_Uuid;
        cass_int64_t u_Bucket;
        std::string u_Password;

        static int selectByDomainAndUsername(CassSession *session, const char *domain, const char *username, UserQuickAccess &target);
    };
};