/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "commands.src.hpp"

namespace serverCommand
{
    std::tuple<SMTPServerCommand, std::string> parse(char *buf)
    {
        /*
        // The current argument
        std::string argumentResult;
        // The current command
        SMTPServerCommand commandResult = SMTPServerCommand::INVALID;
        // The char buffer
        const char *cBuffer = buffer.c_str();
        // Checks the first char to speed stuff up
        if (cBuffer[0] == 'M')
        { // First letter is: M
            if (buffer.substr(0, 9).compare("MAIL FROM") == 0)
            { // Command: MAIL FROM
                commandResult = SMTPServerCommand::MAIL_FROM;
            }
        } else if (cBuffer[0] == 'R')
        { // First letter is: R
            if (buffer.substr(0, 7).compare("RCPT TO") == 0)
            { // Command: RCPT TO
                commandResult = SMTPServerCommand::RCPT_TO;
            }
        } else if (cBuffer[0] == 'D')
        { // First letter is: D
            if (buffer.substr(0, 4).compare("DATA") == 0)
            { // Command: DATA
                commandResult = SMTPServerCommand::DATA;
            }
        } else if (cBuffer[0] == 'S')
        { // First letter is: D
            if (buffer.substr(0, 8).compare("STARTTLS") == 0)
            { // Command: STARTTLS
                commandResult = SMTPServerCommand::START_TLS;
            }
        } else if (cBuffer[0] == 'H')
        { // First letter is: H
            if (buffer.substr(0, 4).compare("HELO") == 0)
            { // Command: HELO
                commandResult = SMTPServerCommand::HELLO;
            } else if (buffer.substr(0, 4).compare("HELP") == 0)
            { // Command: HELP
                commandResult = SMTPServerCommand::HELP;
            }
        } else if (cBuffer[0] == 'E')
        { // First letter is: H
            if (buffer.substr(0, 4).compare("EHLO") == 0)
            { // Command: EHLO
                commandResult = SMTPServerCommand::HELLO;
            }
        } else if (cBuffer[0] == 'Q')
        { // First letter is: X
            if (buffer.substr(0, 4).compare("QUIT") == 0) { // Command: RCPT TO
                commandResult = SMTPServerCommand::QUIT;
            }
        }

        // Checks if the arguments should be parsed
        if (commandResult == SMTPServerCommand::HELLO || commandResult == SMTPServerCommand::MAIL_FROM || commandResult == SMTPServerCommand::RCPT_TO)
        {
            // Parses the arguments
            int argumentStart = buffer.find_first_of(":");
            // Checks if there is an argument
            if (argumentStart != std::string::npos)
            {
                argumentStart++;
                // Gets the specific part of the buffer,
                // and removes the \r\n
                std::string arguments = buffer.substr(argumentStart, buffer.length() - argumentStart - 2);
                // Sets the result
                argumentResult = arguments;
            } else if (commandResult == SMTPServerCommand::HELLO) {
                // Gets the arguments string,
                // and removes the \r\n
                std::string arguments = buffer.substr(5, buffer.length() - 5 - 2);
                // Sets the result
                argumentResult = arguments;
            }
        }

         */

        std::string argumentResult;
        serverCommand::

        // Returns the command
        return std::make_tuple(commandResult, argumentResult);
    }

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
     * Generates response message including code
     * @param code
     * @param param
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