/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "commands.src.hpp"

namespace serverCommand
{
    std::tuple<SMTPServerCommand, std::string> parse(const std::string& buffer)
    {
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
            { // Command: RCPT TO
                commandResult = SMTPServerCommand::DATA;
            }
        } else if (cBuffer[0] == 'S')
        { // First letter is: D
            if (buffer.substr(0, 8).compare("START_TLS") == 0)
            { // Command: RCPT TO
                commandResult = SMTPServerCommand::START_TLS;
            }
        } else if (cBuffer[0] == 'H')
        { // First letter is: H
            if (buffer.substr(0, 4).compare("HELO") == 0)
            { // Command: RCPT TO
                commandResult = SMTPServerCommand::HELLO;
            }
        } else if (cBuffer[0] == 'E')
        { // First letter is: H
            if (buffer.substr(0, 4).compare("EHLO") == 0)
            { // Command: RCPT TO
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

    std::string generate(int code, const char *param)
    {
        // Creates the result stream with the number
        std::ostringstream result;
        // Appends the space
        result << std::to_string(code) << ' ';
        // Checks what should be appended
        switch (code)
        {
            // Introduction
            case 220: {
                result << param << ' ' << "SMTP - Fannst SMTP Server";
                break;
            };
            // Continue, param based
            case 250: {
                result << param;
                break;
            }
            // Data intro
            case 354: {
                result << "End data with <CR><LF>.<CR><LF>";
                break;
            }
            // Exit requested
            case 221: {
                result << "Bye";
                break;
            }
            // Syntax error
            case 501: {
                result << "Syntax error, goodbye !";
                break;
            }
            // Bad sequence
            case 503: {
                result << "Bad sequence, send " << param << " first";
                break;
            }
            // Mail server error
            case 471: {
                result << "Mail server error, " << param;
                break;
            }
            // User not found
            case 551: {
                result << "User not local";
                break;
            }
            // Programmer messed up
            default: {
                result << "Server does not recognize current code";
                break;
            }
        }
        // Returns the result
        return result.str();
    }
};