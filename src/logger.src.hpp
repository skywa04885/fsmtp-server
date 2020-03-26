/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace logger
{
    // This enum holds a logger level,
    // this means the color, and label
    typedef enum {
        LOGGER_DEBUG = 0,
        LOGGER_INFO,
        LOGGER_WARNING,
        LOGGER_ERROR,
        LOGGER_FATAL
    } Level;

    // This enum holds logger options,
    // for example endl
    typedef enum {
        ENDL
    } ConsoleOptions;

    // Logger to console output
    class Console
    {
    public:
        Console(const Level& c_Level, const char *c_Prefix);
        ~Console();

        void setLevel(const Level& c_Level);

        Console& operator << (std::string in)
        {
            this->c_Temp.append(in);
            return *this;
        }

        Console& operator << (int in)
        {
            this->c_Temp.append(std::to_string(in));
            return *this;
        }

        Console& operator << (unsigned int in)
        {
            this->c_Temp.append(std::to_string(in));
            return *this;
        }

        Console& operator << (float in)
        {
            this->c_Temp.append(std::to_string(in));
            return *this;
        }

        Console& operator << (ConsoleOptions opt)
        {
            // Checks which option has been selected
            switch (opt)
            {
                case ConsoleOptions::ENDL:
                {
                    // Holds the message
                    std::ostringstream stream("");
                    // Appends the prefix
                    stream << "\033[36m" << this->c_Prefix << "\033[0m: ";
                    // Creates another switch statement for the current debug level
                    switch (this->c_Level)
                    {
                        case Level::LOGGER_DEBUG:
                        {
                            stream << "\033[35m[DEBUG]\033[0m: ";
                            break;
                        }
                        case Level::LOGGER_INFO:
                        {
                            stream << "\033[34m[INFO]\033[0m: ";
                            break;
                        }
                        case Level::LOGGER_WARNING:
                        {
                            stream << "\033[33m[WARN]\033[0m: ";
                            break;
                        }
                        case Level::LOGGER_ERROR:
                        {
                            stream << "\033[32m[ERROR]\033[0m: ";
                            break;
                        }
                        case Level::LOGGER_FATAL:
                        {
                            stream << "\033[31m[FATAL]\033[0m: ";
                            break;
                        }
                    }
                    // Appends the message to the stream
                    stream << this->c_Temp;
                    // Prints the message
                    std::cout << stream.str() << std::endl;
                    break;
                };
            }
            // Clears the current string
            this->c_Temp.clear();
            return *this;
        }
    private:
        std::string c_Temp;
        Level c_Level;
        const char *c_Prefix;
    };
};