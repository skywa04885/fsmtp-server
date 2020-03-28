/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "cassandra.src.hpp"

namespace cassandra
{
    void cassLoggerErrorHandler(CassFuture *future, logger::Console& print)
    {
        const char *message;
        size_t message_len;

        // Gets the error message
        cass_future_error_message(future, &message, &message_len);

        // Logs the error message
        logger::Level lvl = print.getLevel();
        print.setLevel(logger::Level::LOGGER_ERROR);
        print << "Cassandra Error: " << message << logger::ConsoleOptions::ENDL;
        print.setLevel(lvl);
    }

    Connection::Connection(const char *c_Hosts, bool& success, logger::Console& print):
        c_Hosts(c_Hosts)
    {
        this->c_Session = cass_session_new();
        this->c_Cluster = cass_cluster_new();

        // Sets the contact points for the cluster
        cass_cluster_set_application_version(this->c_Cluster, "3.4.4");
        cass_cluster_set_contact_points(this->c_Cluster, this->c_Hosts);

        // Connects cassandra
        this->c_ConnectFuture = cass_session_connect_keyspace(this->c_Session, this->c_Cluster, "fmail");

        // Waits for connect
        cass_future_wait(this->c_ConnectFuture);

        // Checks if connected successfully
        if (cass_future_error_code(this->c_ConnectFuture) == CASS_OK)
        {
            success = true;
        } else
        {
            cassLoggerErrorHandler(this->c_ConnectFuture, print);
            success = false;
        }
    }

    Connection::~Connection()
    {
        // Closes the session
        cass_session_close(this->c_Session);

        // Frees the memory
        cass_future_free(this->c_ConnectFuture);
        cass_cluster_free(this->c_Cluster);
        cass_session_free(this->c_Session);
    }
};
