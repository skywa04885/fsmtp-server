/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "commands.src.hpp"

namespace Fannst::FSMTPServer::ServerCommand
{
    /**
     * Parses an mailer to server command
     * @param buf
     * @return
     */
    std::tuple<SMTPServerCommand, const char *> parse(char *buf)
    {
        ServerCommand::SMTPServerCommand commandResult = SMTPServerCommand::INVALID;

        // ----
        // Checks which command it is
        // ----

        char *temp = reinterpret_cast<char *>(malloc(16));
        int commandLen = 0;
        if (buf[0] == 'H' || buf[0] == 'E')
        {
            // Copies the memory
            memcpy(&temp[0], &buf[0], 4);
            temp[4] = '\0';

            // Checks what it is
            if (strcmp(&temp[0], "EHLO") == 0 || strcmp(&temp[0], "HELO") == 0)
            {
                commandResult = SMTPServerCommand::HELLO;
                commandLen = 4;
            } else if (strcmp(&temp[0], "HELP") == 0)
            {
                commandResult = ServerCommand::SMTPServerCommand::HELP;
                commandLen = 4;
            }
        } else if (buf[0] == 'D')
        {
            // Copies the memory
            memcpy(&temp[0], &buf[0], 4);
            temp[4] = '\0';

            // Checks what it is
            if (strcmp(&temp[0], "DATA") == 0)
            {
                commandResult = SMTPServerCommand::DATA;
                commandLen = 4;
            }
        } else if (buf[0] == 'M')
        {
            // Copies the memory
            memcpy(&temp[0], &buf[0], 9);
            temp[9] = '\0';

            // Checks what it is
            if (strcmp(&temp[0], "MAIL FROM") == 0)
            {
                commandResult = SMTPServerCommand::MAIL_FROM;
                commandLen = 9;
            }
        } else if (buf[0] == 'R')
        {
            // Copies the memory
            memcpy(&temp[0], &buf[0], 7);
            temp[7] = '\0';

            // Checks what it is
            if (strcmp(&temp[0], "RCPT TO") == 0)
            {
                commandResult = SMTPServerCommand::RCPT_TO;
                commandLen = 7;
            }
        } else if (buf[0] == 'Q')
        {
            // Copies the memory
            memcpy(&temp[0], &buf[0], 4);
            temp[4] = '\0';

            // Checks what it is
            if (strcmp(&temp[0], "QUIT") == 0)
            {
                commandResult = SMTPServerCommand::QUIT;
                commandLen = 4;
            }
        } else if (buf[0] == 'S')
        {
            // Copies the memory
            memcpy(&temp[0], &buf[0], 8);
            temp[8] = '\0';

            // Checks what it is
            if (strcmp(&temp[0], "STARTTLS") == 0)
            {
                commandResult = SMTPServerCommand::START_TLS;
                commandLen = 8;
            }
        }
        // Free's the memory
        delete temp;
        // Checks if there are arguments
        if (buf[commandLen] == ':' || commandResult == SMTPServerCommand::HELLO) {
            // Gets the total amount of bytes to copy
            unsigned long total2copy = strlen(&buf[0]) - commandLen - 2;

            // Reserves space for the memory
            char *arguments = reinterpret_cast<char *>(malloc(total2copy));

            // Copies the memory
            memcpy(&arguments[0], &buf[commandLen], total2copy);
            arguments[total2copy] = '\0';

            // ----
            // Prepares the arguments for usage
            // ----

            // Removes the ':'
            if (arguments[0] == ':') memmove(&arguments[0], &arguments[1], strlen(&arguments[0]));

            // Checks if there is whitespace which needs to be removed
            if (arguments[0] == ' ') memmove(&arguments[0], &arguments[1], strlen(&arguments[0]));

            return std::make_tuple(commandResult, reinterpret_cast<const char *>(arguments));
        } else
        { // No arguments
            return std::make_tuple(commandResult, nullptr);
        }
    }

    /**
     * Converts command enum to string
     * @param command
     * @return
     */
    const char *serverCommandToString(const SMTPServerCommand& command)
    {
        switch (command)
        {
            case SMTPServerCommand::HELLO: {
                return "Hello";
            }
            case SMTPServerCommand::DATA: {
                return "Data";
            }
            case SMTPServerCommand::MAIL_FROM: {
                return "Mail from";
            }
            case SMTPServerCommand::RCPT_TO: {
                return "Rcpt to";
            }
            case SMTPServerCommand::START_TLS: {
                return "Start TLS";
            }
            case SMTPServerCommand::QUIT: {
                return "Quit";
            }
            default: {
                return "Unknown";
            }
        };
    }

    /**
     * Generates an response with code
     * @param code
     * @param param
     * @param listParams
     * @param listParamsN
     * @return
     */
    char *gen(int code, const char *param, const char *listParams[], char listParamsN)
    {
        fcb result = nullptr;
        fcb temp = nullptr;

        // ----
        // Prepares the strings
        // ----

        result = fcb_new(3);

        // ----
        // Appends the code
        // ----

        // Adds the code
        fcb_strcat(result, code);

        /*
        char *result = nullptr;
        char *temp = nullptr;

        // ----
        // Prepares the strings
        // ----

        // Allocates the memory
        result = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(1, 0)));

        // Adds the zero termination char
        result[0] = '\0';

        // ----
        // Appends the code
        // ----

        // Allocates small buffer
        temp = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(3, 0)));

        // Turns the int to an code
        sprintf(&temp[0], "%d", code);

        // Appends the buffer
        strcat(&result[0], temp);

        // ----
        // Appends the space or dash
        // ----

        // Checks if there is an param, and an space needs to be appended
        if (param != nullptr && listParams == nullptr) strcat(&result[0], " ");
        else if (listParams != nullptr) strcat(&result[0], "-");

        // ----
        // Appends the message based on the code
        // ----

        if (param != nullptr) strcat(&result[0], &param[0]);

        return result;


        // ----
        // Adds ESMTP options if needed
        // ----

        // Checks if any arguments need to be added
        if (listParamsN > 0)
        {
            // Adds the <CR><LF>
            strcat(&result[0], "\r\n");

            // Loops over the params
            for (char i = 0; i < listParamsN; i++)
            {
                // Adds the code
                strcat(&result[0], &temp[0]);

                // Checks if there needs to be added an dash or space
                if (i+1 == listParamsN) strcat(&result[0], " ");
                else strcat(&result[0], "-");

                // Appends the params
                strcat(&result[0], &listParams[i][0]);

                // Appends <CR><LF>
                strcat(&result[0], "\r\n");
            }
        } else strcat(result, "\r\n");

        // ----
        // Frees
        // ----

        // Frees the buffer
        free(temp);

        // ----
        // Returns the result
        // ----

        return result;
        */
    }
};