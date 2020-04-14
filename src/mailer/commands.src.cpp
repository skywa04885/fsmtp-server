#include "commands.src.hpp"

namespace Fannst
{
    int parseCommand(const char *raw, int& code, char *arguments)
    {
        char codeString[4];

        // Parses the code from the message
        memcpy(&codeString[0], &raw[0], 3);
        codeString[3] = '\0';

        // Parses the code to an integer
        code = atoi(&codeString[0]);

        // ----
        // Parses the arguments
        // ----

        // Gets the message without the number
        strncpy(&arguments[0], &raw[3], strlen(&raw[0]) - 5);
        arguments[strlen(&arguments[0])] = '\0';

        // Checks if there is an whitespace in the begin, if so .. Remove it
        if (arguments[0] == ' ') memmove(&arguments[0], &arguments[1], strlen(&arguments[0]));

        return 0;
    }

    const char *gen(const ClientCommand &clientCommand, const bool& argsInline, const char *args)
    {
        char *target = nullptr;

        // Checks if there are args, if so allocate extra memory for them
        if (args == nullptr) target = reinterpret_cast<char *>(malloc(24));
        else target = reinterpret_cast<char *>(malloc(strlen(&args[0]) + 24));

        // Adds the string end
        target[0] = '\0';

        // Checks which type of command it is, and adds the text
        switch (clientCommand)
        {
            case ClientCommand::CM_HELO:
            {
                strcat(&target[0], "HELO");
                break;
            }

            case ClientCommand::CM_QUIT:
            {
                strcat(&target[0], "QUIT");
                break;
            }

            case ClientCommand::CM_DATA:
            {
                strcat(&target[0], "DATA");
                break;
            }

            case ClientCommand::CM_MAIL_TO:
            {
                strcat(&target[0], "RCPT TO");
                break;
            }

            case ClientCommand::CM_MAIL_FROM:
            {
                strcat(&target[0], "MAIL FROM");
                break;
            }

            case ClientCommand::CM_START_TLS:
            {
                strcat(&target[0], "STARTTLS");
                break;
            }
        }

        // Checks if there are arguments, if so append an ' ' or ": "
        if (args == nullptr)
        {
            // Adds the CR LF
            strcat(&target[0], "\r\n");

            // Returns the result
            return const_cast<const char *>(target);
        }

        // Adds the separator
        if (argsInline) strcat(&target[0], " ");
        else strcat(&target[0], ": ");

        // Adds the arguments itself
        strcat(&target[0], args);

        // Adds the CR LF
        strcat(&target[0], "\r\n");

        // Returns the result
        return const_cast<const char *>(target);
    }
}
