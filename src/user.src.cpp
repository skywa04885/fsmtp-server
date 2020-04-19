/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "user.src.hpp"
#include "db/cassandra.src.hpp"

namespace Fannst::FSMTPServer::Models
{
    User::User(
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
    ):
        u_FullName(u_FullName),
        u_Password(u_Password),
        u_Domain(u_Domain),
        u_Username(u_Username),
        u_uuid(u_uuid),
        u_Description(u_Description),
        u_Picture(u_Picture),
        u_BirthDate(u_BirthDate),
        u_CreationDate(u_CreationDate),
        u_Bucket(u_Bucket)
    {}

    UserQuickAccess::UserQuickAccess(
        const char *u_Domain,
        const char *u_Username,
        const CassUuid &u_Uuid,
        const cass_int64_t &u_Bucket,
        const char *u_Password
    ):
        u_Domain(u_Domain),
        u_Username(u_Username),
        u_Uuid(u_Uuid),
        u_Bucket(u_Bucket),
        u_Password(u_Password)
    {}

    int UserQuickAccess::selectByDomainAndUsername(CassSession *session, const char *domain, const char *username, UserQuickAccess& target) {
        // Creates the query
        const char *query = "SELECT u_uuid, u_bucket, u_password FROM users_quick_access WHERE u_domain = ? AND u_username = ?";

        // Creates the statement
        CassStatement *statement = cass_statement_new(query, 2);

        // Prepares the values
        cass_statement_bind_string(statement, 0, domain);
        cass_statement_bind_string(statement, 1, username);

        // Executes the query
        CassFuture *query_future = cass_session_execute(session, statement);

        // Waits for the statement to be executed
        cass_future_wait(query_future);

        // Checks if there was an error
        if (cass_future_error_code(query_future) != CASS_OK)
        {
            const char *message;
            size_t message_len;
            cass_future_error_message(query_future, &message, &message_len);
            std::cerr << "user.src.cpp: " << message << std::endl;
            return -1;
        }

        // Gets the results
        const CassResult *result = cass_future_get_result(query_future);
        CassIterator *iterator = cass_iterator_from_result(result);

        // Checks if the row is empty
        if (!cass_iterator_next(iterator))
        {
            return -2;
        }

        // Gets the row
        const CassRow *row = cass_iterator_get_row(iterator);

        // The result variables
        size_t resPasswordLen;
        CassUuid res_uuid;
        cass_int64_t res_bucket;
        const char *res_password;

        // Receives the data from database
        cass_value_get_string(cass_row_get_column_by_name(row, "u_password"), &res_password, &resPasswordLen);
        cass_value_get_int64(cass_row_get_column_by_name(row, "u_bucket"), &res_bucket);
        cass_value_get_uuid(cass_row_get_column_by_name(row, "u_uuid"), &res_uuid);

        // Creates the QuickUserAccess instance
        target.u_Bucket = res_bucket;

        target.u_Domain = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(strlen(&domain[0]), 0)));
        memcpy(&const_cast<char *>(target.u_Domain)[0], &domain[0], strlen(&domain[0]) + 1);

        target.u_Password = reinterpret_cast<char *>(malloc(ALLOCATE_NULL_TERMINATION(resPasswordLen)));
        memset(&const_cast<char *>(target.u_Password)[resPasswordLen], '\0', 1);
        memcpy(&const_cast<char *>(target.u_Password)[0], &res_password[0], resPasswordLen);

        target.u_Username = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(strlen(&username[0]), 0)));
        memcpy(&const_cast<char *>(target.u_Username)[0], &username[0], strlen(&username[0]) + 1);

        target.u_Uuid = res_uuid;

        // Frees the memory
        cass_future_free(query_future);
        cass_statement_free(statement);
        cass_result_free(result);
        cass_iterator_free(iterator);

        // Returns with code success
        return 0;
    }

    UserQuickAccess::UserQuickAccess() {}
};