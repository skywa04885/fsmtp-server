#pragma once

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fannst_libcompose.hpp>

#include "../logger.src.hpp"
#include "dkim/dkim.src.hpp"
#include "resolver.src.hpp"
#include "commands.src.hpp"
#include "socket-handler.hpp"

namespace Fannst
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

    class Mailer
    {
    public:
        /**
         * Constructor for the mailer
         * @param c_ComposerOptions
         */
        explicit Mailer(Fannst::Composer::Options &c_ComposerOptions);

        /**
         * Sends an email
         * @return
         */
        int sendMessage(const char *extIp);
    private:
        Fannst::Composer::Options &c_ComposerOptions;
    };

    int transmitMessage(char *ipAddress, Fannst::Types::EmailAddress &mailFrom,
                        Fannst::Types::EmailAddress &mailTo, std::string &messageBody, bool usingSSL,
                        const char *extIp);
};