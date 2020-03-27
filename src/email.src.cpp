/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "email.src.hpp"

namespace models
{
    int Email::save() {
        const char *query = "INSERT INTO inbox_def ("
                            "e_transport_from, e_transport_to, e_transport_address,"
                            "e_mail_from, e_mail_to, e_mail_subject"
                            ")";

        return 0;
    }
};
