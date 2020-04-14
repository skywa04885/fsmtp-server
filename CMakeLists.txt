cmake_minimum_required(VERSION 3.0)
project(fsmtp_server)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

add_library(
        models STATIC

        src/email.src.hpp
        src/email.src.cpp

        src/user.src.hpp
        src/user.src.cpp
)

add_library(
        db STATIC

        src/db/cassandra.src.hpp
        src/db/cassandra.src.cpp
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
        src/server/responses.src.hpp
        src/queued-email.src.cpp
        src/queued-email.src.hpp
        src/pre.hpp
)

add_library(
        mailer STATIC

        src/mailer/mailer.src.cpp
        src/mailer/mailer.src.hpp
        src/mailer/resolver.src.hpp
        src/mailer/resolver.src.cpp
        src/mailer/commands.src.hpp
        src/mailer/commands.src.cpp
        src/mailer/socket-handler.cpp
        src/mailer/socket-handler.hpp
)

add_executable(fsmtp_server main.cpp)
target_link_libraries(fsmtp_server mailer server db logger models -lpthread -lfannst_libcompose -lfannst_libencoding -lcassandra -lresolv -lssl -lcrypto ${GTK3_LIBRARIES})