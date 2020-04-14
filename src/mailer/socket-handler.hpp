#pragma once

#include <tuple>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fannst_libcompose.hpp>

#include "pre.hpp"
#include "commands.src.hpp"

namespace Fannst::FSMTPClient::SocketHandler
{
    // ----
    // The functions which will send or receive data
    // ----

    inline int read(const int *soc, SSL *ssl, char *buf, const int &n)
    {
        int rc;

        // Clears the buffer
        memset(&buf[0], 0, n);

        // Checks if we're using tls
        if (ssl == nullptr)
        { // Using plain text

            // Reads into the buffer
            rc = recv(*soc, &buf[0], n, 0);

            // Checks if there were any errors
            if (rc < 0)
                FSMT_PREP_ERROR("Plain Text Socket Error", "Could not read data")
        } else
        { // Using START TLS

            // Reads into the buffer
            rc = SSL_read(ssl, &buf[0], n);

            // Checks if there were any errors
            if (rc <= 0)
            {
                FSMT_PREP_ERROR("SSL Socket Error", "Could not read data")
                ERR_print_errors_fp(stderr);
            }
        }

        FSMTP_PREP_DEBUG_PRINT("Received from server", buf);

        // Returns the code
        return rc;
    }

    /**
     * Writes either to an START TLS socket, or plain text socket
     * @param soc
     * @param ssl
     * @param buf
     * @param n
     * @return
     */
    inline int write(const int *soc, SSL *ssl, const char *buf, const int &n)
    {
        int rc;

        // Checks if we're using tls
        if (ssl == nullptr)
        { // Using plain text

            // Writes to the client
            rc = send(*soc, &buf[0], n, 0);

            // Checks if there were any errors
            if (rc < 0)
                FSMT_PREP_ERROR("Plain Text Socket Error", "Could not write data")
        } else
        { // Using Start TLS

            // Writes to the client
            rc = SSL_write(ssl, &buf[0], n);

            // Checks if there were any errors
            if (rc <= 0)
            {
                FSMT_PREP_ERROR("SSL Socket Error", "Could not write data")
                ERR_print_errors_fp(stderr);
            }
        }

        FSMTP_PREP_DEBUG_PRINT("Sent to server", buf);

        // Returns the code
        return rc;
    }

    // ----
    // The writing methods
    // ----

    bool handleHelo(const int *soc, SSL *ssl);

    /**
     * Handles mail to
     * @param soc
     * @param ssl
     * @param options
     * @return
     */
    bool handleMailTo(const int *soc, SSL *ssl, fannst::composer::Options &options);


    /**
     * Handles mail to
     * @param soc
     * @param ssl
     * @param options
     * @return
     */
    bool handleMailTo(const int *soc, SSL *ssl, fannst::types::EmailAddress &target);

    /**
     * Handles mail from
     * @param soc
     * @param ssl
     * @param options
     * @return
     */
    bool handleMailFrom(const int *soc, SSL *ssl, fannst::types::EmailAddress &target);

    /**
     * Sends data command
     * @param soc
     * @param ssl
     * @return
     */
    bool handleDataCommand(const int *soc, SSL *ssl);

    /**
     * Appends an <CR><LF>.<CR><LF> to message, and transmits it
     * @param soc
     * @param ssl
     * @param message
     * @param n
     * @return
     */
    bool handleDataTransmission(const int *soc, SSL *ssl, const char *message, const int &n);

    /**
     * Disconnects from the server
     * @param soc
     * @param ssl
     * @return
     */
    bool handleQuit(const int *soc, SSL *ssl);

    /**
     * Sends the 'STARTTLS' command
     * @param soc
     * @param ssl
     * @return
     */
    bool handleStartTlsCommand(const int *soc, SSL *ssl);
}
