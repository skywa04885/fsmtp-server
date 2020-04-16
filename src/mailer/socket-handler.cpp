#include "socket-handler.hpp"

namespace Fannst::FSMTPClient::SocketHandler
{
    // ----
    // The writing methods
    // ----

    bool handleHelo(const int *soc, SSL *ssl, const char *ipAddress)
    {
        // ----
        // Formats the address
        // ----

        // Generates the temp buffer for message
        char *st = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(strlen(&ipAddress[0]), 2)));
        st[0] = '\0';

        // Appends the stuff
        strcat(&st[0], "[");
        strcat(&st[0], &ipAddress[0]);
        strcat(&st[0], "]");

        // ----
        // Transmits the message
        // ----

        // Generates the response message
        const char *message = Fannst::gen(Fannst::ClientCommand::CM_HELO, true, st);

        // Deletes tge temp string
        free(st);

        // Writes the message
        if (write(soc, ssl, &message[0], strlen(&message[0])) < 0)
        {
            delete message;
            return false;
        } else delete message;

        // Returns true, keep socket open
        return true;
    }

    /**
     * Handles mail to
     * @param soc
     * @param ssl
     * @param options
     * @return
     */
    bool handleMailTo(const int *soc, SSL *ssl, Fannst::Types::EmailAddress &target)
    {
        // ----
        // Generates the address
        // ----
        const char *addrRaw = target.e_Address.c_str();
        char *addrResult = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(strlen(&addrRaw[0] + 2), 0)));
        addrResult[0] = '\0';
        strcat(&addrResult[0], "<");
        strcat(&addrResult[0], &addrRaw[0]);
        strcat(&addrResult[0], ">");

        // ----
        // Generates the response
        // ----
        const char *message = Fannst::gen(Fannst::ClientCommand::CM_MAIL_TO, false, addrResult);
        delete addrResult;


        // Writes the response message
        if (write(soc, ssl, &message[0], strlen(&message[0])) < 0)
        {
            delete message;
            return false;
        } else delete message;

        // Returns true, keep connection open
        return true;
    }

    /**
     * Handles mail from
     * @param soc
     * @param ssl
     * @param options
     * @return
     */
    bool handleMailFrom(const int *soc, SSL *ssl, Fannst::Types::EmailAddress &target) {
        // ----
        // Generates the address
        // ----
        const char *addrRaw = target.e_Address.c_str();
        char *addrResult = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(strlen(&addrRaw[0] + 2), 0)));
        addrResult[0] = '\0';
        strcat(&addrResult[0], "<");
        strcat(&addrResult[0], &addrRaw[0]);
        strcat(&addrResult[0], ">");

        // ----
        // Generates the response
        // ----
        const char *message = Fannst::gen(Fannst::ClientCommand::CM_MAIL_FROM, false, addrResult);
        delete addrResult;

        // Writes the response message
        if (write(soc, ssl, &message[0], strlen(&message[0])) < 0) {
            delete message;
            return false;
        } else delete message;

        // Returns true, keep connection open
        return true;
    }

    /**
     * Sends data command
     * @param soc
     * @param ssl
     * @return
     */
    bool handleDataCommand(const int *soc, SSL *ssl)
    {
        // Generates the response message
        const char *message = Fannst::gen(Fannst::ClientCommand::CM_DATA, false,
                nullptr);

        // Writes the response message
        if (write(soc, ssl, &message[0], strlen(&message[0])) < 0) {
            delete message;
            return false;
        } else delete message;

        // Returns true, keep connection open
        return true;
    }

    /**
     * Appends an <CR><LF>.<CR><LF> to message, and transmits it
     * @param soc
     * @param ssl
     * @param message
     * @param n
     * @return
     */
    bool handleDataTransmission(const int *soc, SSL *ssl, const char *message, const int &n)
    {
        // Reserves the memory for the message, message length + 5
        char *messageReady = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(n, 5)));
        messageReady[0] = '\0';

        // Copies the string into the message
        strcpy(&messageReady[0], &message[0]);

        // Appends the <CR><LF>.<CR><LF>
        strcat(&messageReady[0], "\r\n.\r\n");

        // Writes the response message
        if (write(soc, ssl, &messageReady[0], strlen(&messageReady[0])) < 0) {
            delete messageReady;
            return false;
        } else delete messageReady;

        // Returns true, keep connection open
        return true;
    }

    /**
     * Disconnects from the server
     * @param soc
     * @param ssl
     * @return
     */
    bool handleQuit(const int *soc, SSL *ssl)
    {
        // Generates the response message
        const char *message = Fannst::gen(Fannst::ClientCommand::CM_QUIT, false,
                                          nullptr);

        // Writes the response message
        if (write(soc, ssl, &message[0], strlen(&message[0])) < 0) {
            delete message;
            return false;
        } else delete message;

        // Returns true, keep connection open
        return true;
    }

    /**
     * Sends the 'STARTTLS' command
     * @param soc
     * @param ssl
     * @return
     */
    bool handleStartTlsCommand(const int *soc, SSL *ssl)
    {
        // Generates the response message
        const char *message = Fannst::gen(Fannst::ClientCommand::CM_START_TLS, false,
                                          nullptr);

        // Writes the response message
        if (write(soc, ssl, &message[0], strlen(&message[0])) < 0) {
            delete message;
            return false;
        } else delete message;

        // Returns true, keep connection open
        return true;
    }
}