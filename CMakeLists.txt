cmake_minimum_required(VERSION 3.15)
project(fsmtp_server)

set(CMAKE_CXX_STANDARD 20)

add_library(
        models STATIC

        src/email.src.hpp
        src/email.src.cpp
)

add_library(
        logger STATIC

        src/logger.src.hpp
        src/logger.src.cpp
)

add_library(
        server STATIC

        src/server/commands.src.hpp
        src/server/commands.src.cpp
        src/server/server.src.hpp
        src/server/server.src.cpp
        src/parsers/mail-parser.src.cpp
        src/parsers/mail-parser.src.hpp
)

add_executable(fsmtp_server main.cpp)
target_link_libraries(fsmtp_server server logger models -lpthread)