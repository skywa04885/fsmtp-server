/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

#include <chrono>
#include <iostream>

class Timer
{
public:
    Timer(const char *t_Label): t_Label(t_Label)
    {
        // Gets the current time
        std::chrono::high_resolution_clock::time_point now =
                std::chrono::high_resolution_clock::now();

        // Stores the current time in ms
        this->t_Start = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

        // Prints the start
        std::cout << "\033[36m ~ \033[34mTimer@" << this->t_Label << ":\033[0m started .." << std::endl;
    }

    ~Timer()
    {
        // Gets the current time
        std::chrono::high_resolution_clock::time_point now =
                std::chrono::high_resolution_clock::now();

        // Gets the time difference in ms
        long df = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count() - this->t_Start;

        // Prints the time difference
        std::cout << "\033[36m ~ \033[34mTimer@" << this->t_Label << ":\033[0m finished in " << df << "ns, "
        << df / 1000.0f << "μs" << ", " << df / 100000.0f << "ms" << std::endl;
    }
private:
    const char *t_Label;
    long t_Start;
};