/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "sender.src.hpp"

namespace Fannst::FSMTPServer::Sender
{
    /**
     * Runs the email sender queue as thread
     * @param argc
     * @param argv
     */
    void run(int *argc, char ***argv)
    {
        std::vector<Models::QueuedEmail> queuedEmails{};
        bool cassConnSuccess;
        std::size_t messageSendIndex;

        // Creates the logger
        Logger::Console print(Logger::Level::LOGGER_DEBUG, "Sender Queue");

        // The infinite loop
        for (;;)
        {
            // Prints the initial message
            DEBUG_ONLY(print << "Checking for messages to send ..." << Logger::ConsoleOptions::ENDL)

            // ----
            // Creates an new cassandra connection
            // ----

            // Performs an connection attempt
            Cassandra::Connection cassConnection(GE_CASSANDRA_CONTACT_POINTS, cassConnSuccess);

            // Checks if the connection was successfully
            if (!cassConnSuccess)
            {
                // Sets the logger level
                print.setLevel(Logger::LOGGER_ERROR);

                // Prints the error
                print << "Could not connect to Apache Cassandra." << Logger::ConsoleOptions::ENDL;

                // Restores the old level
                print.setLevel(Logger::LOGGER_DEBUG);
            }

            // ----
            // Receives the messages that have to be sent
            // ----

            // Gets the emails
            Models::QueuedEmail::get(cassConnection.c_Session, queuedEmails, 10);

            // Checks if there is anything to be transmitted
            if (!queuedEmails.empty())
            {
                // Prints to the console
                DEBUG_ONLY(print << "Started sending [" << queuedEmails.size() << "] emails ..." << Logger::ConsoleOptions::ENDL)

                // ----
                // Starts looping over the messages to be sent
                // ----

                // Starts the loop
                messageSendIndex = 0;
                for (Models::QueuedEmail &qm : queuedEmails)
                {
                    // ----
                    // Gets the email details
                    // ----

                    // Creates the message
                    Models::Email email{};

                    // Gets the message from the database
                    Models::Email::getMessage(cassConnection.c_Session, qm.m_Bucket, qm.m_UserUUID, qm.m_UUID, email);

                    // Prints the message if requested
                    DEBUG_ONLY(print << "Sending from: " << email.m_TransportFrom.e_Address << ", to: "
                        << email.m_TransportTo.e_Address << Logger::ConsoleOptions::ENDL)

                    // ----
                    // Creates the mime-composer options, and sends the email
                    // ----
//                    // Generates the options for the mime-composer
//                    Composer::Options composerOptions{};
//                    composerOptions.o_Domain = GE_DOMAIN;
//
//                    // Sends the message
//                    SMTPMailer mailer();

                    // Increments the index
                    messageSendIndex++;
                }

                // Prints to the console
                DEBUG_ONLY(print << "Sent [" << queuedEmails.size() << "] emails ;)" << Logger::ConsoleOptions::ENDL)
            }

            // ----
            // Resets the required variables
            // ----

            queuedEmails.clear();

            // ----
            // Waits some time before going to the next queue iteration
            // ----

            std::this_thread::sleep_for(std::chrono::seconds(9));
        }
    }
}