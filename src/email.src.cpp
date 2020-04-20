/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "email.src.hpp"
#include "db/cassandra.src.hpp"

namespace Fannst::FSMTPServer::Models {
    static const CassSchemaMeta *cassSchemaMeta = nullptr;

    /**
     * Saves an email inside cassandra
     * @param session
     * @return
     */
    int Email::save(CassSession *session) {
        // Generates the bukket
        this->m_Bucket = Email::getCurrentBucket();

        /**
         * 1. Initializes the types
         * 2. Create the m_transport_to
         * 3. Create the m_transport_from
         * 4. Create the m_from
         * 5. Create the m_to
         * 6. Create the m_full_headers
         * 7. Create the m_content
         * 8. Stores the email
         */

        /**
         * 1. Initializes the types
         */

        // Initializes the variables
        const CassDataType *udt_email_address;
        const CassDataType *udt_email_content_section;
        const CassDataType *udt_email_header;

        // Defines them based on the keyspace meta
        const CassSchemaMeta *schemaMeta = cass_session_get_schema_meta(session);
        const CassKeyspaceMeta *keyspaceMeta = cass_schema_meta_keyspace_by_name(schemaMeta, "fmail");
        if (keyspaceMeta == nullptr) {
            return -1;
        }
        udt_email_address = cass_keyspace_meta_user_type_by_name(keyspaceMeta, "email_address");
        udt_email_content_section = cass_keyspace_meta_user_type_by_name(keyspaceMeta, "email_content_section");
        udt_email_header = cass_keyspace_meta_user_type_by_name(keyspaceMeta, "email_header");


        /**
         * 2. Creates the m_transport_to
         */

        // Creates the variable
        CassUserType *m_transport_to = cass_user_type_new_from_data_type(udt_email_address);

        // Assigns the values
        cass_user_type_set_string_by_name(m_transport_to, "e_name", this->m_TransportTo.e_Name);
        cass_user_type_set_string_by_name(m_transport_to, "e_address", this->m_TransportTo.e_Address);

        /**
         * 3. Creates the m_transport_from
         */

        // Creates the variable
        CassUserType *m_transport_from = cass_user_type_new_from_data_type(udt_email_address);

        // Assigns the values
        cass_user_type_set_string_by_name(m_transport_from, "e_name", this->m_TransportFrom.e_Name);
        cass_user_type_set_string_by_name(m_transport_from, "e_address", this->m_TransportFrom.e_Address);

        /**
         * 4. Creates the m_from
         */

        // Creates the m_from list
        CassCollection *m_from = cass_collection_new(CASS_COLLECTION_TYPE_LIST, this->m_From.size());

        // Loops over the addresses and appends them
        for (const auto &address : this->m_From) {
            CassUserType *m_from_temp = cass_user_type_new_from_data_type(udt_email_address);

            // Sets the values
            cass_user_type_set_string_by_name(m_from_temp, "e_name", address.e_Name);
            cass_user_type_set_string_by_name(m_from_temp, "e_address", address.e_Address);

            // Appends it to the list
            cass_collection_append_user_type(m_from, m_from_temp);

            // Frees the memory
            cass_user_type_free(m_from_temp);
        }

        /**
         * 5. Creates the m_to
         */

        // Creates the m_to list
        CassCollection *m_to = cass_collection_new(CASS_COLLECTION_TYPE_LIST, this->m_To.size());

        // Loops over the addresses and appends them
        for (const auto &address : this->m_To) {
            CassUserType *m_to_temp = cass_user_type_new_from_data_type(udt_email_address);

            // Sets the values
            cass_user_type_set_string_by_name(m_to_temp, "e_name", address.e_Name);
            cass_user_type_set_string_by_name(m_to_temp, "e_address", address.e_Address);

            // Appends it to the list
            cass_collection_append_user_type(m_to, m_to_temp);

            // Frees the memory
            cass_user_type_free(m_to_temp);
        }

        /**
         * 6. Creates the m_full_headers
         */

        // Creates the m_full_headers list
        CassCollection *m_full_headers = cass_collection_new(CASS_COLLECTION_TYPE_LIST, this->m_FullHeaders.size());

        for (const auto &header : this->m_FullHeaders) {
            // Loops over the headers and appends them
            CassUserType *m_full_headers_temp = cass_user_type_new_from_data_type(udt_email_header);

            // Sets the values
            cass_user_type_set_string_by_name(m_full_headers_temp, "e_key", header.h_Key);
            cass_user_type_set_string_by_name(m_full_headers_temp, "e_value", header.h_Value);

            // Appends it to the list
            cass_collection_append_user_type(m_full_headers, m_full_headers_temp);

            // Frees the memory
            cass_user_type_free(m_full_headers_temp);
        }

        /**
         * 7. Creates the m_content
         */

        // Creates the m_content list
        CassCollection *m_content = cass_collection_new(CASS_COLLECTION_TYPE_LIST, this->m_Content.size());

        for (const auto &section : this->m_Content) {
            // Creates the section
            CassUserType *m_content_section_temp = cass_user_type_new_from_data_type(udt_email_content_section);

            // Creates the section headers
            CassCollection *m_content_section_headers_temp = cass_collection_new(CASS_COLLECTION_TYPE_LIST,
                                                                                 section.s_FullHeaders.size());

            // Loops over the section headers
            for (const auto &header : section.s_FullHeaders) {
                // Creates the header temp
                CassUserType *m_content_section_header_temp = cass_user_type_new_from_data_type(udt_email_header);

                // Sets the variables
                cass_user_type_set_string_by_name(m_content_section_header_temp, "e_key", header.h_Key);
                cass_user_type_set_string_by_name(m_content_section_header_temp, "e_value", header.h_Value);

                // Appends to the collection
                cass_collection_append_user_type(m_content_section_headers_temp, m_content_section_header_temp);

                // Frees the memory
                cass_user_type_free(m_content_section_header_temp);
            }

            // Sets the variables
            cass_user_type_set_collection_by_name(m_content_section_temp, "e_full_headers",
                                                  m_content_section_headers_temp);
            cass_user_type_set_string_by_name(m_content_section_temp, "e_content", section.s_Content);
            cass_user_type_set_int32_by_name(m_content_section_temp, "e_index", section.s_Index);
            cass_user_type_set_int32_by_name(m_content_section_temp, "e_type", section.s_ContentType);

            // Appends to the result collection
            cass_collection_append_user_type(m_content, m_content_section_temp);

            // Frees memory
            cass_user_type_free(m_content_section_temp);
            cass_collection_free(m_content_section_headers_temp);
        }

        /**
         * 8. Stores the email
         */

        // Creates the query
        const char *query = "INSERT INTO inbox_emails ("
                            "m_transport_to, m_transport_from, m_subject,"
                            "m_message_id, m_date, m_boundary,"
                            "m_content_type, m_timestamp, m_receive_timestamp,"
                            "m_from, m_to, m_full_headers, m_content,"
                            "m_bucket, m_uuid, m_user_uuid"
                            ") VALUES ("
                            "?, ?, ?, ?, ?,"
                            "?, ?, ?, ?, ?,"
                            "?, ?, ?, ?, ?,"
                            "?"
                            ")";

        // Creates the statement
        CassStatement *statement = cass_statement_new(query, 16);

        // Binds the values
        cass_statement_bind_user_type(statement, 0, m_transport_to);
        cass_statement_bind_user_type(statement, 1, m_transport_from);
        cass_statement_bind_string(statement, 2, this->m_Subject);
        cass_statement_bind_string(statement, 3, this->m_MessageID);
        cass_statement_bind_int64(statement, 4, 500);
        cass_statement_bind_string(statement, 5, this->m_Boundary);
        cass_statement_bind_int32(statement, 6, this->m_ContentType);
        cass_statement_bind_int64(statement, 7, this->m_Timestamp);
        cass_statement_bind_int64(statement, 8, this->m_ReceiveTimestamp);
        cass_statement_bind_collection(statement, 9, m_from);
        cass_statement_bind_collection(statement, 10, m_to);
        cass_statement_bind_collection(statement, 11, m_full_headers);
        cass_statement_bind_collection(statement, 12, m_content);
        cass_statement_bind_int64(statement, 13, this->m_Bucket);
        cass_statement_bind_uuid(statement, 14, this->m_UUID);
        cass_statement_bind_uuid(statement, 15, this->m_UserUUID);

        // Executes the query
        CassFuture *query_future = cass_session_execute(session, statement);
        cass_future_wait(query_future);

        // Clears the memory
        cass_user_type_free(m_transport_to);
        cass_user_type_free(m_transport_from);
        cass_collection_free(m_from);
        cass_collection_free(m_to);
        cass_collection_free(m_content);
        cass_collection_free(m_full_headers);
        cass_statement_free(statement);
        cass_schema_meta_free(schemaMeta);

        // Checks if it went fine
        if (cass_future_error_code(query_future) == CASS_OK) {
            // ----
            // Frees the memory
            // ----

            cass_future_free(query_future);
            return 0;
        } else {
            const char *message;
            size_t message_len;
            cass_future_error_message(query_future, &message, &message_len);
            std::cerr << "email.src.cpp: " << message << std::endl;
            cass_future_free(query_future);
        }

        return -2;
    }

    /**
     * Gets the current email bucket
     */
    long Email::getCurrentBucket() {
        // Gets the current time point
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        // Calculates the milliseconds sinds somewhere in 1970
        long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        // Calculates the bucket, basically new one every 18 hours
        return std::round(now_ms / 1000 / 60 / 60 / 18);
    }

    /**
     * Gets an email by uuid
     * @param cassSession
     * @param uuid
     * @param target
     * @return
     */
    BYTE Email::getMessage(CassSession *cassSession, long bucket, CassUuid userUuid, CassUuid &uuid, Email &target)
    {
        CassStatement *statement = nullptr;
        CassFuture *future = nullptr;
        CassError erc;
        BYTE rc = 0;

        // ----
        // If not there, gets the scheme meta
        // ----

        // ----
        // Generates the query
        // ----

        const char *query = "SELECT * FROM fmail.inbox_emails WHERE m_bucket = ? AND m_user_uuid = ? AND m_uuid = ?";

        // ----
        // Prepares the statement
        // ----

        // Creates the statement
        statement = cass_statement_new(query, 3);

        // Binds the values to the statement
        cass_statement_bind_int64(statement, 0, bucket);
        cass_statement_bind_uuid(statement, 1, userUuid);
        cass_statement_bind_uuid(statement, 2, uuid);

        // ----
        // Executes the statement
        // ----

        // Executes the statement
        future = cass_session_execute(cassSession, statement);

        // Waits for the statement to be executed
        cass_future_wait(future);

        // Checks if the execution was successful
        erc = cass_future_error_code(future);
        if (erc != CassError::CASS_OK)
        {
            // Gets the error
            const char *message;
            size_t message_len;
            cass_future_error_message(future, &message, &message_len);
            PREP_ERROR("Cassandra Error", message)

            // Sets the return code
            rc = -1;
        } else
        {
            const CassResult *result = nullptr;
            CassIterator *iterator = nullptr;

            // Gets the result
            result = cass_future_get_result(future);

            // Creates the iterator
            iterator = cass_iterator_from_result(result);

            // Checks if there is any data
            if (cass_iterator_next(iterator))
            {
                const CassRow *row = nullptr;
                std::size_t tempLen;
                const char *mSubject = nullptr;
                const char *mMessageID = nullptr;
                const char *mBoundary = nullptr;
                int tempInt;

                // ----
                // Prepares the data gathering
                // ----

                // Gets the row
                row = cass_iterator_get_row(iterator);

                // ----
                // Gets all the string values
                // ----

                // - m_subject

                // Gets the value
                cass_value_get_string(cass_row_get_column_by_name(row, "m_subject"), &mSubject, &tempLen);
                // Allocate the required space in the target
                target.m_Subject = reinterpret_cast<const char *>(malloc(ALLOCATE_NULL_TERMINATION(tempLen)));
                // Copies the memory value
                memcpy(&const_cast<char *>(target.m_Subject)[0], &mSubject[0], tempLen + 1);

                // - m_message_id

                // Gets the value
                cass_value_get_string(cass_row_get_column_by_name(row, "m_message_id"), &mMessageID, &tempLen);
                // Allocate the required space in the target
                target.m_MessageID = reinterpret_cast<const char *>(malloc(ALLOCATE_NULL_TERMINATION(tempLen)));
                // Copies the memory value
                memcpy(&const_cast<char *>(target.m_MessageID)[0], &mMessageID[0], tempLen + 1);

                // - m_boundary

                // Gets the value
                cass_value_get_string(cass_row_get_column_by_name(row, "m_boundary"), &mBoundary, &tempLen);
                // Allocate the required space in the target
                target.m_Boundary = reinterpret_cast<const char *>(malloc(ALLOCATE_NULL_TERMINATION(tempLen)));
                // Copies the memory value
                memcpy(&const_cast<char *>(target.m_Boundary)[0], &mBoundary[0], tempLen + 1);

                // ----
                // Gets all the int values
                // ----

                // - m_date
                cass_value_get_int64(cass_row_get_column_by_name(row, "m_date"), &target.m_Date);
                // - m_content_type
                cass_value_get_int32(cass_row_get_column_by_name(row, "m_content_type"), &tempInt);
                target.m_ContentType = static_cast<Types::MimeContentType>(tempInt);
                // - m_timestamp
                cass_value_get_int64(cass_row_get_column_by_name(row, "m_timestamp"), &target.m_Timestamp);
                // - m_receive_timestamp
                cass_value_get_int64(cass_row_get_column_by_name(row, "m_receive_timestamp"), &target.m_ReceiveTimestamp);
                // - m_bucket
                cass_value_get_int64(cass_row_get_column_by_name(row, "m_bucket"), &target.m_Bucket);

                // ----
                // Gets the UUID's
                // ----

                // - m_uuid
                cass_value_get_uuid(cass_row_get_column_by_name(row, "m_uuid"), &target.m_UUID);
                // - m_user_uuid
                cass_value_get_uuid(cass_row_get_column_by_name(row, "m_user_uuid"), &target.m_UserUUID);

                // ----
                // Gets the m_transport_to
                // ----


                Cassandra::Helpers::getEmailAddressFromColumn(
                        cass_row_get_column_by_name(row, "m_transport_to"), target.m_TransportTo);

                // ----
                // Gets the m_transport_from
                // ----

                Cassandra::Helpers::getEmailAddressFromColumn(
                        cass_row_get_column_by_name(row, "m_transport_From"), target.m_TransportFrom);

                // ----
                // Gets the m_from
                // ----

                {
                    CassIterator *collectionFromIt = nullptr;
                    const CassValue *collectionFrom = nullptr;

                    // ----
                    // Gets the iterator as result of collection type list
                    // ----

                    // Gets the column
                    collectionFrom = cass_row_get_column_by_name(row, "m_from");

                    // Creates the iterator
                    collectionFromIt = cass_iterator_from_collection(collectionFrom);

                    // ----
                    // Starts the iteration
                    // ----

                    while (cass_iterator_next(collectionFromIt))
                    {
                        // Creates the temp address
                        Types::EmailAddress a{};

                        // Gets the value, and stores it in the temp address
                        Cassandra::Helpers::getEmailAddressFromColumn(cass_iterator_get_value(collectionFromIt), a);

                        // Stores the temp address inside of the "m_from" vector of the target
                        target.m_From.push_back(a);
                    }

                    // ----
                    // Frees the memory
                    // ----

                    cass_iterator_free(collectionFromIt);
                }

                // ----
                // Gets the m_to
                // ----

                {
                    CassIterator *collectionToIt = nullptr;
                    const CassValue *collectionTo = nullptr;

                    // ----
                    // Gets the iterator as result of collection type list
                    // ----

                    // Gets the column
                    collectionTo = cass_row_get_column_by_name(row, "m_to");

                    // Creates the iterator
                    collectionToIt = cass_iterator_from_collection(collectionTo);

                    // ----
                    // Starts the iteration
                    // ----

                    while (cass_iterator_next(collectionToIt))
                    {
                        // Creates the temp address
                        Types::EmailAddress a{};

                        // Gets the value, and stores it in the temp address
                        Cassandra::Helpers::getEmailAddressFromColumn(cass_iterator_get_value(collectionToIt), a);

                        // Stores the temp address inside of the "m_from" vector of the target
                        target.m_From.push_back(a);
                    }

                    // ----
                    // Frees the memory
                    // ----

                    cass_iterator_free(collectionToIt);
                }

                // ----
                // Gets the header
                // ----

                {
                    CassIterator *collectionHeaderIt = nullptr;
                    const CassValue *headerVal = nullptr;

                    // ----
                    // Prepares the iterator
                    // ----

                    // Gets our column
                    headerVal = cass_row_get_column_by_name(row, "m_full_headers");

                    // Creates the iterator
                    collectionHeaderIt = cass_iterator_from_collection(headerVal);

                    // ----
                    // Starts iterating
                    // ----

                    while (cass_iterator_next(collectionHeaderIt))
                    {
                        // Creates the temp header
                        Types::MimeHeader h{
                            nullptr,
                            nullptr,
                            true
                        };

                        // Gets the header values
                        Cassandra::Helpers::getHeadersFromColumn(cass_iterator_get_value(collectionHeaderIt), h);

                        // Pushes it to the result vector
                        target.m_FullHeaders.push_back(h);
                    }

                    // ----
                    // Frees the memory
                    // ----

                    cass_iterator_free(collectionHeaderIt);
                }

                // ----
                // Gets the m_content
                // ----

                {
                    CassIterator *collectionSectionIt = nullptr;
                    const CassValue *sectionValue = nullptr;

                    // ----
                    // Prepares the iterator
                    // ----

                    // Gets the value
                    sectionValue = cass_row_get_column_by_name(row, "m_content");

                    // Creates the iterator
                    collectionSectionIt = cass_iterator_from_collection(sectionValue);

                    // ----
                    // Starts iterating
                    // ----

                    // Iterators
                    while (cass_iterator_next(collectionSectionIt))
                    {
                        Types::MimeBodySection s{};
                        const CassValue *sectionValue = nullptr;
                        CassIterator *valueFields = nullptr;

                        // ----
                        // Prepares
                        // ----

                        // Gets the section value
                        sectionValue = cass_iterator_get_value(collectionSectionIt);

                        // Creates the fields iterator
                        valueFields = cass_iterator_fields_from_user_type(sectionValue);

                        // ----
                        // Starts the iteration
                        // ----

                        while (cass_iterator_next(valueFields))
                        {
                            const char *fieldName = nullptr;
                            const CassValue *fieldValue = nullptr;
                            std::size_t fieldLen;

                            // ----
                            // Gets the value and name
                            // ----

                            // Gets the field name
                            cass_iterator_get_user_type_field_name(valueFields, &fieldName, &fieldLen);

                            // Gets the field value
                            fieldValue = cass_iterator_get_user_type_field_value(valueFields);

                            // ----
                            // Checks which field it is
                            // ----

                            if (fieldName[2] == 'c' && strcmp(&fieldName[0], "e_content") == 0)
                            { // Is "e_content"

                                const char *eContentTemp = nullptr;

                                // Gets the string
                                cass_value_get_string(fieldValue, &eContentTemp, &tempLen);

                                // Allocates the memory for the copy
                                s.s_Content = reinterpret_cast<char *>(malloc(ALLOCATE_NULL_TERMINATION(tempLen)));

                                // Copies the memory
                                memcpy(&const_cast<char *>(s.s_Content)[0], &eContentTemp[0], tempLen + 1);
                            } else if (fieldName[2] == 't' && strcmp(&fieldName[0], "e_type") == 0)
                            { // Is "e_type"

                                // Gets the temp int
                                cass_value_get_int32(fieldValue, &tempInt);

                                // Stores the content type in the result
                                s.s_ContentType = static_cast<Types::MimeContentType>(tempInt);
                            } else if (fieldName[2] == 'f' && strcmp(&fieldName[0], "e_full_headers") == 0)
                            { // Is "e_full_headers"
                                CassIterator *fieldHeadersValueIt = nullptr;

                                // ----
                                // Prepares the iterator
                                // ----

                                // Gets the value
                                fieldHeadersValueIt = cass_iterator_from_collection(fieldValue);

                                // ----
                                // Starts looping over the fields
                                // ----

                                while (cass_iterator_next(fieldHeadersValueIt))
                                {
                                    Types::MimeHeader h{
                                        nullptr,
                                        nullptr,
                                        true
                                    };

                                    // Parses the header
                                    Cassandra::Helpers::getHeadersFromColumn(
                                            cass_iterator_get_value(fieldHeadersValueIt), h);

                                    // Pushes the header to the final result
                                    s.s_FullHeaders.push_back(h);
                                }

                                // ----
                                // Frees the memory
                                // ----

                                cass_iterator_free(fieldHeadersValueIt);
                            } else if (fieldName[2] == 'i' && strcmp(&fieldName[0], "e_index") == 0)
                            { // Is "e_index"

                                // Gets the field index
                                cass_value_get_int32(fieldValue, &s.s_Index);
                            } else PREP_ERROR("Field name not recognized, please fix ;)", fieldName);
                        }

                        // ----
                        // Pushes the result to the target
                        // ----

                        target.m_Content.push_back(s);

                        // ----
                        // Frees the memory
                        // ----

                        cass_iterator_free(valueFields);
                    }

                    // ----
                    // Frees the memory
                    // ----

                    cass_iterator_free(collectionSectionIt);
                }
            }

            // ----
            // Frees the memory
            // ----

            cass_iterator_free(iterator);
            cass_result_free(result);
        }

        // ----
        // Frees the memory
        // ----

        cass_future_free(future);
        cass_statement_free(statement);

        return rc;
    }
};
