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
        } else if (buf[0] == 'A')
        {
            // Copies the memory
            memcpy(&temp[0], &buf[0], 4);
            temp[4] = '\0';

            // Checks what it is
            if (strcmp(&temp[0], "AUTH") == 0)
            {
                commandResult = SMTPServerCommand::AUTH;
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
        // Free's the temp memory
        free(temp);

        // Checks if there need to be default argument type parsed
        if (buf[commandLen] == ':' || commandResult == SMTPServerCommand::AUTH
        || commandResult == SMTPServerCommand::HELLO) {
            // ----
            // Checks if there is any content at all
            // ----

            if (strlen(&buf[commandLen]) == 2) return std::make_tuple(commandResult, nullptr);

            // ----
            // Copies the memory
            // ----

            // Gets the size to copy
            std::size_t size2copy = ALLOC_CAS_STRING(strlen(&buf[0]) - commandLen - 1, 0);
            // Allocate the memory
            char *args = reinterpret_cast<char *>(malloc(size2copy));
            args[0] = '\0';
            // Copies the required memory
            strcat(&args[0], &buf[commandLen+1]);
            // Removes the <CR><LF>
            args[size2copy - 3] = '\0';

            // Resize the buffer
            size2copy -= 2;
            args = reinterpret_cast<char *>(realloc(&args[0], size2copy));

            // ----
            // If whitespace at begin, remove it
            // ----

            if (args[0] == ' ') memmove(&args[0], &args[1], strlen(&args[0]));

            // ----
            // Returns the result
            // ----
            // std::cout << "'" << args << "'" << std::endl;

            return std::make_tuple(commandResult, reinterpret_cast<const char *>(args));
        } else return std::make_tuple(commandResult, nullptr);
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
        char *result = nullptr;
        std::size_t resultSize;

        // ----
        // Prepares the strings
        // ----

        // Creates the buffer with 3 chars available for the code, and one for the dash or space
        result = reinterpret_cast<char *>(malloc(ALLOC_CAS_STRING(12, 0)));
        result[0] = '\0';
        resultSize = 13;

        // ----
        // Appends the code
        // ----

        // Creates the code buffer
        char buffer[12];
        // Turns the code into an string
        sprintf(&buffer[0],"%d", code);

        // Adds the code
        strcat(&result[0], &buffer[0]);

        // ----
        // Adds '-' or ' '
        // ----

        if (param != nullptr && listParams == nullptr) strcat(&result[0], " ");
        else if (listParams != nullptr) strcat(&result[0], "-");

        // ----
        // Adds params if there
        // ----

        // Prepares an param value
        if (param == nullptr) param = "No message specified";

        // Resizing the buffer, plus two for <CR><LF>
        resultSize += strlen(&param[0]) + 2;
        result = reinterpret_cast<char *>(realloc(&result[0], resultSize));

        // Concats the params
        strcat(&result[0], &param[0]);

        // ----
        // Adds arguments if required
        // ----

        // Checks if any arguments need to be added
        if (listParamsN > 0 && listParams != nullptr)
        {
            // Adds the <CR><LF>
            strcat(&result[0], "\r\n");

            // Loops over the arguments
            for (char i = 0; i < listParamsN; i++)
            {

                // Resizing the buffer, for the code, one dash and two for <CR><LF>
                resultSize += strlen(listParams[i]) + 6;
                result = reinterpret_cast<char *>(realloc(&result[0], resultSize));

                // Appends the code
                strcat(&result[0], &buffer[0]);

                // Checks if dash or space needs to be appended
                if (i+1 != listParamsN) strcat(&result[0], "-");
                else strcat(&result[0], " ");

                // Appends the param
                strcat(&result[0], listParams[i]);
                // Appends the <CR><LF>
                strcat(&result[0], "\r\n");
            }
        } else strcat(&result[0], " - fsmtp\r\n");

        return result;
    }
};