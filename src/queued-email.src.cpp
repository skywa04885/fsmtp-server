/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "queued-email.src.hpp"

namespace Fannst::FSMTPServer::Models
{
    /**
     * Default constructor for an queued email
     * @param m_timestamp
     * @param m_uuid
     */
    QueuedEmail::QueuedEmail(const long &m_timestamp, const CassUuid &m_uuid):
        m_timestamp(m_timestamp), m_uuid(m_uuid) {}

    /**
     * Saves an queued email
     * @param session
     * @return
     */
    BYTE QueuedEmail::save(CassSession *session) {
        BYTE rc = 0;

        CassStatement *statement = nullptr;
        CassFuture *future = nullptr;
        CassError erc;

        // ----
        // Creates the query
        // ----

        // Creates the query
        const char *query = "INSERT INTO fmail.queued_emails ("
                            "m_timestamp, m_uuid"
                            ") VALUES ("
                            "?, ?"
                            ")";

        // ----
        // Prepares the statement
        // ----

        // Creates the statement
        statement = cass_statement_new(query, 2);

        // Binds the values
        cass_statement_bind_int64(statement, 0, this->m_timestamp);
        cass_statement_bind_uuid(statement, 1, this->m_uuid);

        // ----
        // Executes the query
        // ----

        // Executes the query
        future = cass_session_execute(session, statement);

        // Waits for the query to be executed
        cass_future_wait(future);

        // Checks if the query was executed
        erc = cass_future_error_code(future);
        if (erc != CassError::CASS_OK)
        {
            // Gets the error message
            const char *message = nullptr;
            std::size_t messageLen;
            cass_future_error_message(future, &message, &messageLen);
            PREP_ERROR("Cassandra Error", message)

            // Sets the return code
            rc = -1;
        }

        // ----
        // Frees the memory
        // ----

        cass_future_free(future);
        cass_statement_free(statement);

        return rc;
    }

    /**
     * Gets message to sent using the queue
     * @param cassSession
     * @param result
     * @param count
     * @return
     */
    BYTE QueuedEmail::get(CassSession *cassSession, std::vector<QueuedEmail> &result, const int &count)
    {
        CassStatement *statement = nullptr;
        CassFuture *future = nullptr;
        BYTE rc = 0;
        CassError erc;

        // ----
        // Creates the query
        // ----

        const char *query = "SELECT m_uuid, m_timestamp FROM fmail.queued_emails LIMIT ?";

        // ----
        // Creates the statement
        // ----

        // Creates the statement
        statement = cass_statement_new(query, 1);

        // Binds the values
        cass_statement_bind_int32(statement, 0, count);

        // ----
        // Executes the query, and gets the results
        // ----

        // Execute
        future = cass_session_execute(cassSession, statement);

        // Waits for the query to finish
        cass_future_wait(future);

        // Checks if it was successful
        erc = cass_future_error_code(future);
        if (erc != CassError::CASS_OK)
        {
            // Gets the error message
            const char *message = nullptr;
            std::size_t messageLen;
            cass_future_error_message(future, &message, &messageLen);
            PREP_ERROR("Cassandra Error", message)

            // Sets the return code
            rc = -1;
        } else
        {
            const CassResult *cassResult = nullptr;
            CassIterator *iterator = nullptr;

            // Gets the result
            cassResult = cass_future_get_result(future);
            // Creates the iterator
            iterator = cass_iterator_from_result(cassResult);

            // Checks if there are any results to begin with
            while (cass_iterator_next(iterator))
            {
                const CassRow *row = nullptr;
                CassUuid m_uuid{};
                cass_int64_t m_timestamp{};

                // ----
                // Processes the data
                // ----

                // Gets the current row
                row = cass_iterator_get_row(iterator);

                // Gets the values
                cass_value_get_uuid(cass_row_get_column_by_name(row, "m_uuid"), &m_uuid);
                cass_value_get_int64(cass_row_get_column_by_name(row, "m_timestamp"), &m_timestamp);

                // ----
                // Pushes the values to the result vector
                // ----

                result.emplace_back(QueuedEmail(m_timestamp, m_uuid));
            }

            // Frees the memory
            cass_result_free(cassResult);
            cass_iterator_free(iterator);
        }

        // ----
        // Frees the memory
        // ----

        cass_statement_free(statement);
        cass_future_free(future);

        return rc;
    }
}