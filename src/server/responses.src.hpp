/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <openssl/ssl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./commands.src.hpp"
#include "../pre.hpp"
#include "../email.src.hpp"
#include "../parsers/mail-parser.src.hpp"
#include "server.src.hpp"
#include "../user.src.hpp"

namespace Fannst::FSMTPServer::Responses
{
    // ----
    // The global methods, such as write
    // ----

    /**
     * Writes an message to an encrypted socket or plain text socket
     * @param soc
     * @param ssl
     * @param msg
     * @param msg_len
     */
    void write(const int *soc, SSL *ssl, const char *msg, int msg_len)
    {
        if (ssl == nullptr)
        {
            if (send(*soc, msg, msg_len, 0) < 0) {
                PREP_ERROR("Could not send message", msg)
            }
        } else
        {
            // Sends the message over the socket
            SSL_write(ssl, msg, msg_len);

            // TODO: Check for errors
        }
    }

    // ----
    // The pre-programmed responses
    // ----

    /**
     * Sends that the mailer may continue
     * @param ssl
     */
    void preContextProceed(int *soc, SSL *ssl)
    {
        char *message = ServerCommand::gen(250, "OK Proceed", nullptr, 0);
        write(soc, ssl, message, strlen(message));
        free(message);
    }

    /**
     * Sends that an bad sequence occurred
     * @param ssl
     * @param reqMsg
     */
    void preContextBadSequence(int *soc, SSL *ssl, const char *reqMsg)
    {
        char *message = ServerCommand::gen(503, reqMsg, nullptr, 0);
        write(soc, ssl, message, strlen(message));
        free(message);
    }

    /**
     * Sends an syntax error
     * @param ssl
     */
    void syntaxError(int *soc, SSL *ssl)
    {
        char *message = ServerCommand::gen(501, "Syntax error", nullptr, 0);
        write(soc, ssl, message, strlen(message));
        free(message);
    }

    // ----
    // The request handlers
    // ----

    bool handleMailFrom(int *soc, SSL *ssl, const char *args, Models::Email &email, Server::ConnPhasePT &phase)
    {
        // Checks if the sequence is correct
        if (phase < Server::ConnPhasePT::PHASE_PT_HELLO)
        {
            // Writes the error
            preContextBadSequence(soc, ssl, "HELO");
            // Returns true, so mailer can retry
            return true;
        }

        // Checks if the args are empty, if so return syntax error
        if (args == nullptr || strlen(args) == 0)
        {
            // Writes the syntax error
            syntaxError(soc, ssl);
            // Returns false, close connection
            return false;
        }

        // Parses the email address from the args, if invalid: send syntax error
        if (Parsers::parseAddress(args, email.m_TransportFrom) < 0)
        {
            // Writes the syntax error
            syntaxError(soc, ssl);
            // Returns false, close connection
            return false;
        }

        // Checks if the sender is a person from Fannst, and possibly tries to relay message
        // TODO: Relay check

        // Sends continue
        preContextProceed(soc, ssl);

        // Sets the phase
        phase = Server::ConnPhasePT::PHASE_PT_MAIL_FROM;

        // Returns, everything went fine
        return true;
    }

    bool handleHelo(int *soc, SSL *ssl, const char *args, Server::ConnPhasePT &phase, struct sockaddr_in *sockaddrIn)
    {
        // ----
        // Checks if args are there
        // ----

        if (args == nullptr || strlen(args) == 0)
        {
            // Sends the response
            char *message = ServerCommand::gen(501, "Empty EHLO/HELO command not allowed, closing connection.", nullptr, 0);
            write(soc, ssl, message, strlen(message));
            free(message);

            // Closes the connection
            return false;
        }

        // ----
        // Generates the message title
        // ----

        char *temp = reinterpret_cast<char *>(malloc(64));
        temp[0] = '\0';
        strcat(&temp[0], "smtp.fannst.nl at your service, [");
        strcat(&temp[0], inet_ntoa(sockaddrIn->sin_addr));
        strcat(&temp[0], "]");

        // ----
        // Sends the message
        // ----

        const char *argList[] {"STARTTLS", "HELP"};
        char *message = ServerCommand::gen(250, &temp[0], argList, 2);
        write(soc, ssl, message, strlen(message));
        free(message);

        // Updates the phase
        phase = Server::ConnPhasePT::PHASE_PT_HELLO;

        // Returns true
        return true;
    }

    void handleQuit(int *soc, SSL *ssl)
    {
        // Sends the response message
        char *message = ServerCommand::gen(221, "Goodbye !", nullptr, 0);
        write(soc, ssl, message, strlen(message));
        free(message);
    }

    bool handleRcptTo(int *soc, SSL *ssl, const char *args, Models::Email &email, Server::ConnPhasePT &phase,
                      CassSession *cassSession)
    {
        // Checks if the sequence is correct
        if (phase < Server::ConnPhasePT::PHASE_PT_MAIL_FROM)
        {
            // Writes the error
            preContextBadSequence(soc, ssl, "MAIL FROM");
            // Returns false
            return true;
        }

        // Checks if the arguments are empty
        if (args == nullptr || strlen(args) == 0)
        {
            // Writes an syntax error
            syntaxError(soc, ssl);
            // Returns false
            return false;
        }

        // Parses the address
        if (Parsers::parseAddress(args, email.m_TransportTo) < 0)
        {
            // Writes the syntax error
            syntaxError(soc, ssl);
            // Returns false, close connection
            return false;
        }

        // Splits the address
        char *emailCstr = const_cast<char *>(email.m_TransportTo.e_Address.c_str());
        char *username = nullptr;
        char *domain = nullptr;
        char *tok = strtok(emailCstr, "@");

        // Checks if the string is too long, and if we need to stop
        if (strlen(emailCstr) > 256)
        {
            PREP_ERROR("Refused parsing", "Address longer then 256 chars, preventing stack overflow ..")

            // Sends the error message
            char *message = ServerCommand::gen(471,
                                                     "Address too large, would cause stack overflow ..", nullptr, 0);
            write(soc, ssl, message, strlen(message));
            free(message);

            // Returns false
            return false;
        }

        // Loop over the occurrences, char because of the small size
        unsigned char i = 0;
        while (tok != nullptr)
        {
            if (i == 0)
            {
                // Allocates space on the stack
                username = reinterpret_cast<char *>(alloca(strlen(tok)));
                // Copies the string, including the  '\0'
                memcpy(username, tok, strlen(tok) + 1);
            } else if (i == 1)
            {
                // Allocates space on the stack
                domain = reinterpret_cast<char *>(alloca(strlen(tok)));
                // Copies the string, including the  '\0'
                memcpy(domain, tok, strlen(tok) + 1);
            } else break;

            // Finds the next token
            tok = strtok(nullptr, "@");
            i++; // Increments i
        }

        // Checks if the parsing went good, if not return server error
        if (username == nullptr || domain == nullptr)
        {
            // Sends the response message
            char *message = ServerCommand::gen(471, "could not parse address ..",
                                                     nullptr, 0);
            write(soc, ssl, message, strlen(message));
            free(message);

            // Returns that there was an error
            return false;
        }

        // ----
        // Performs user check from Apache Cassandra
        // ----

        // Finds the receiver on our server
        Models::UserQuickAccess userQuickAccess;
        int rc = Models::UserQuickAccess::selectByDomainAndUsername(cassSession, domain, username,
                userQuickAccess);

        // Checks if the user was found
        if (rc == -1)
        {
            // Sends the response message
            char *message = ServerCommand::gen(471, "Could not perform cassandra query ..",
                                                     nullptr, 0);
            write(soc, ssl, message, strlen(message));
            free(message);

            // Returns false
            return false;
        } else if (rc == -2)
        {
            // Sends the response message
            char *message = ServerCommand::gen(551, "", nullptr, 0);
            write(soc, ssl, message, strlen(message));
            free(message);

            // Returns false
            return false;
        }

        // Sets the user id to the email user id
        email.m_UserUUID = userQuickAccess.u_Uuid;

        // Sends continue
        preContextProceed(soc, ssl);

        // Sets the phase
        phase = Server::ConnPhasePT::PHASE_PT_MAIL_TO;

        // Returns, everything went fine
        return true;
    }
}