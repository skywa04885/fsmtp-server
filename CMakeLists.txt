cmake_minimum_required(VERSION 3.0)
project(fsmtp_server)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)

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

        src/mailer/dkim/dkim.src.hpp
        src/mailer/dkim/dkim.src.cpp
        src/mailer/dkim/parsers.src.cpp
        src/mailer/dkim/canonicalization/relaxed.src.cpp
        src/mailer/dkim/canonicalization/simple.src.cpp
)

add_library(
        ossl STATIC

        src/ossl/sha1.src.cpp
        src/ossl/sha256.src.cpp
        src/ossl/rsa-sha256.src.cpp
        src/ossl/ossl.src.hpp
)

add_library(
        esmtp_modules STATIC

        src/server/modules/esmtp-auth.src.cpp
        src/server/modules/esmtp-auth.src.hpp
        src/server/modules/esmtp.src.cpp
        src/server/modules/esmtp.src.hpp
)

add_library(
        flib STATIC

        src/flib/strings.hpp
        src/flib/strings.cpp
)

add_executable(fsmtp_server main.cpp)
target_link_libraries(fsmtp_server mailer server esmtp_modules db logger models flib ossl -lpthread -lfannst_libcompose -lfannst_libencoding -lcassandra -lresolv -lssl -lcrypto)