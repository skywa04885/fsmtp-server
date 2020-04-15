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

        char *temp = reinterpret_cast<char *>(malloc(9));
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
    const char *gen(int code, const char *param, const char *listParams[], char listParamsN)
    {

        // ----
        // Prepares the strings
        // ----

        // Prepares the final result
        char *result = reinterpret_cast<char *>(malloc(sizeof(char) * (strlen(param) + 128)));
        result[0] = '\0';               // Sets the string end, strcat will move this to the end

        // Result: Code + WS
        char *temp = reinterpret_cast<char *>(malloc(sizeof(char) * 8));
        sprintf(&temp[0], "%d", code);
        strcat(&result[0], temp);

        // Checks if there is an param, and an space needs to be appended
        if (param != nullptr && listParams == nullptr) strcat(result, " ");
        else if (listParams != nullptr) strcat(result, "-");

        // ----
        // Appends the message based on the code
        // ----

        switch (code)
        {
            // Introduction
            case 220: {
                // TODO: Fix so it will report back correct string for start tls continue
                if (param == nullptr) strcat(&result[0], "UNKNOWN");
                else strcat(&result[0], &param[0]);
                strcat(result, " - ESMTP service ready");
                break;
            };
            // Continue, param based
            case 250: {
                if (param == nullptr) strcat(&result[0], "OK Proceed");
                else strcat(&result[0], &param[0]);
                break;
            }
            // Data intro
            case 354: {
                strcat(result, "End data with <CR><LF>.<CR><LF>");
                break;
            }
            // Exit requested
            case 221: {
                strcat(result, "Bye");
                break;
            }
            // Syntax error
            case 501: {
                if (param == nullptr) strcat(result, "Syntax error.");
                else strcat(result, &param[0]);
                break;
            }
            // Bad sequence
            case 503: {
                strcat(result ,"Bad sequence, send ");
                strcat(result, param);
                strcat(result," first");
                break;
            }
            // Mail server error
            case 471: {
                strcat(result, "Mail server error, ");
                strcat(result, param);
                break;
            }
            // User not found
            case 551: {
                strcat(result, "User not local");
                break;
            }
            // Programmer messed up
            default: {
                strcat(result, "Server does not recognize current code");
                break;
            }
        }

        // ----
        // Adds ESMTP options if needed
        // ----

        // Checks if any arguments need to be added
        if (listParams != nullptr)
        {
            // Adds the <CR><LF>
            strcat(&result[0], "\r\n");

            // Loops over the params
            for (char i = 0; i < listParamsN; i++)
            {
                strcat(&result[0], &temp[0]);
                if (i+1 == listParamsN) strcat(&result[0], " ");
                else strcat(&result[0], "-");
                strcat(&result[0], &listParams[i][0]);
                strcat(&result[0], "\r\n");
            }
        } else
        { // Just add <CR><LF>

            // Adds the <CR><LF>
            strcat(result, "\r\n");
        }

        // Returns the result
        return const_cast<const char *>(result);
    }
};