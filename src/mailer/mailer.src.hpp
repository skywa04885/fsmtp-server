#pragma once

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../logger.src.hpp"
#include "../parsers/mime-parser.src.hpp"
#include "dkim/dkim.src.hpp"
#include "resolver.src.hpp"
#include "commands.src.hpp"
#include "socket-handler.hpp"

namespace Fannst::FSMTPServer::Mailer
{
    typedef enum {
        MST_INITIAL = 0,
        MST_START_TLS,
        MST_START_TLS_CONNECT,
        MST_HELO_START_TLS_START,
        MST_HELO,
        MST_MAIL_FROM,
        MST_MAIL_TO,
        MST_DATA,
        MST_DATA_START,
        MST_DATA_END
    } MailerState;

    class SMTPMailer
    {
    public:
        /**
         * Constructor for the mailer
         * @param c_ComposerOptions
         */

        SMTPMailer(const char *c_Message, const std::vector<Types::EmailAddress> &c_To,
                               const std::vector<Types::EmailAddress> &c_From):
                c_Message(c_Message), c_To(c_To), c_From(c_From)
        {}

        /**
         * Sends an email
         * @return
         */
        int sendMessage(const char *extIp);
    private:
        const char *c_Message{nullptr};
        const std::vector<Types::EmailAddress> &c_To;
        const std::vector<Types::EmailAddress> &c_From;
    };

    int transmitMessage(char *ipAddress, const Types::EmailAddress &mailFrom,
        const Types::EmailAddress &mailTo, const char *messageBody, bool usingSSL,
        const char *extIp);
};