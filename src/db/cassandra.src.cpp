/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "cassandra.src.hpp"

namespace Fannst::FSMTPServer::Cassandra
{

    Connection::Connection(const char *c_Hosts, bool& success):
        c_Hosts(c_Hosts)
    {
        this->c_Session = cass_session_new();
        this->c_Cluster = cass_cluster_new();

        // Sets the contact points for the cluster
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
            const char *message;
            size_t message_len;
            cass_future_error_message(this->c_ConnectFuture, &message, &message_len);
            PREP_ERROR("Cassandra could not initialize connection", message);
            success = false;
        }

        // Frees the memory
        cass_cluster_free(this->c_Cluster);
        cass_future_free(this->c_ConnectFuture);
    }

    Connection::~Connection()
    {
        // Frees the memory
        cass_session_free(this->c_Session);
    }
};
