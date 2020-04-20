/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "mime-helpers.src.hpp"

namespace Fannst::FSMTPServer::Cassandra::Helpers
{
    /**
     * Gets the email address from an column
     * @param value
     * @return
     */
    BYTE getEmailAddressFromColumn(const CassValue *value, Types::EmailAddress &target)
    {
        CassIterator *iterator = nullptr;
        std::size_t tempLen;

        // Creates an iterator for the row
        iterator = cass_iterator_fields_from_user_type(value);

        // ---
        // Starts looping over the udt fields
        // ---

        while (cass_iterator_next(iterator))
        {
            const CassValue *fieldValue = nullptr;
            const char *fieldName = nullptr;
            std::size_t fieldLen;

            // Gets the field name
            cass_iterator_get_user_type_field_name(iterator, &fieldName, &fieldLen);

            // Gets the field value
            fieldValue = cass_iterator_get_user_type_field_value(iterator);

            // Checks which field we are dealing with
            if (fieldName[2] == 'n' && strcmp(&fieldName[0], "e_name") == 0)
            { // Is "e_name"

                const char *eName = nullptr;

                // Gets the value
                cass_value_get_string(fieldValue, &eName, &tempLen);

                // Allocates memory inside of the target for the copy
                target.e_Name = reinterpret_cast<const char *>(malloc(
                        ALLOCATE_NULL_TERMINATION(tempLen)));

                // Copies the data from the eName variable to the result
                memcpy(&const_cast<char *>(target.e_Name)[0], &eName[0], tempLen + 1);
            } else if (fieldName[2] == 'a' && strcmp(&fieldName[0], "e_address") == 0)
            { // Is "e_address"

                const char *eAddress = nullptr;

                // Gets the value
                cass_value_get_string(fieldValue, &eAddress, &tempLen);

                // Allocates memory inside of the target for the copy
                target.e_Address = reinterpret_cast<const char *>(malloc(
                        ALLOCATE_NULL_TERMINATION(tempLen)));

                // Copies the data from the eName variable to the result
                memcpy(&const_cast<char *>(target.e_Address)[0], &eAddress[0], tempLen + 1);
            } else PREP_ERROR("Field name not recognized, please fix ;)", fieldName);
        }

        // ----
        // Frees the memory
        // ----

        cass_iterator_free(iterator);
    }

    /**
     * Gets the header from an column
     * @param value
     * @return
     */
    BYTE getHeadersFromColumn(const CassValue *value, Types::MimeHeader &target)
    {
        CassIterator *iterator = nullptr;
        std::size_t tempLen;

        // Creates an iterator for the row
        iterator = cass_iterator_fields_from_user_type(value);

        // ---
        // Starts looping over the udt fields
        // ---

        while (cass_iterator_next(iterator))
        {
            const CassValue *fieldValue = nullptr;
            const char *fieldName = nullptr;
            std::size_t fieldLen;

            // Gets the field name
            cass_iterator_get_user_type_field_name(iterator, &fieldName, &fieldLen);

            // Gets the field value
            fieldValue = cass_iterator_get_user_type_field_value(iterator);

            // Checks which field we are dealing with
            if (fieldName[2] == 'k' && strcmp(&fieldName[0], "e_key") == 0)
            { // Is "e_name"

                const char *eName = nullptr;

                // Gets the value
                cass_value_get_string(fieldValue, &eName, &tempLen);

                // Allocates memory inside of the target for the copy
                target.h_Key = reinterpret_cast<const char *>(malloc(
                        ALLOCATE_NULL_TERMINATION(tempLen)));

                // Copies the data from the eKey variable to the result
                memcpy(&const_cast<char *>(target.h_Key)[0], &eName[0], tempLen + 1);
            } else if (fieldName[2] == 'v' && strcmp(&fieldName[0], "e_value") == 0)
            { // Is "e_address"

                const char *eAddress = nullptr;

                // Gets the value
                cass_value_get_string(fieldValue, &eAddress, &tempLen);

                // Allocates memory inside of the target for the copy
                target.h_Value = reinterpret_cast<const char *>(malloc(
                        ALLOCATE_NULL_TERMINATION(tempLen)));

                // Copies the data from the eValue variable to the result
                memcpy(&const_cast<char *>(target.h_Value)[0], &eAddress[0], tempLen + 1);
            } else PREP_ERROR("Field name not recognized, please fix ;)", fieldName);
        }

        // ----
        // Frees the memory
        // ----

        cass_iterator_free(iterator);
    }
}