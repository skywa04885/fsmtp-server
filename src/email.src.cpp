/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "email.src.hpp"
#include "db/cassandra.src.hpp"

namespace Fannst::FSMTPServer::Models
{
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
        if (keyspaceMeta == nullptr)
        {
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
        for (const auto &address : this->m_From)
        {
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
        for (const auto &address : this->m_To)
        {
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

        for (const auto &header : this->m_FullHeaders)
        {
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

        for (const auto &section : this->m_Content)
        {
            // Creates the section
            CassUserType *m_content_section_temp = cass_user_type_new_from_data_type(udt_email_content_section);

            // Creates the section headers
            CassCollection *m_content_section_headers_temp = cass_collection_new(CASS_COLLECTION_TYPE_LIST, section.s_FullHeaders.size());

            // Loops over the section headers
            for (const auto &header : section.s_FullHeaders)
            {
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
            cass_user_type_set_collection_by_name(m_content_section_temp, "e_full_headers", m_content_section_headers_temp);
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

        // Checks if it went fine
        if (cass_future_error_code(query_future) == CASS_OK)
        {
            cass_future_free(query_future);
            return 0;
        } else
        {
            const char *message;
            size_t message_len;
            cass_future_error_message(query_future, &message, &message_len);
            std::cerr << "email.src.cpp: " << message << std::endl;
            cass_future_free(query_future);
        }

        return -2;
    }

    long Email::getCurrentBucket() {
        // Gets the current time point
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        // Calculates the milliseconds sinds somewhere in 1970
        long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        // Calculates the bucket, basically new one every 18 hours
        return std::round(now_ms / 1000 / 60 / 60 / 18);
    }
};
