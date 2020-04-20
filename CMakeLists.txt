# Configures CMAKE
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# Sets the C++ Version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configures the project
PROJECT(fsmtp_server C CXX)

# ----
# Compiles the real library's
# ----

# ----
# Compiles the sub libraries
# ----

ADD_LIBRARY(
        lib_db STATIC

        src/db/cassandra.src.hpp
        src/db/cassandra.src.cpp
)

ADD_LIBRARY(
        server STATIC

        # Server itself
        src/server/server.src.hpp
        src/server/server.src.cpp

        # The server message parser
        src/server/parse-message.src.cpp
        src/server/parse-message.src.hpp

        # The server commands
        src/server/commands.src.cpp
        src/server/commands.src.hpp

        # The server response header
        src/server/responses.src.hpp

        # The modules
        src/server/modules/esmtp.src.hpp
        src/server/modules/esmtp.src.cpp

        src/server/modules/esmtp-auth.src.hpp
        src/server/modules/esmtp-auth.src.cpp
)

ADD_LIBRARY(
        models STATIC

        # The email models
        src/email.src.cpp
        src/email.src.hpp

        src/queued-email.src.cpp
        src/queued-email.src.hpp

        src/inbox-emails-quick-access.src.cpp
        src/inbox-emails-quick-access.src.hpp

        # The user
        src/user.src.cpp
        src/user.src.hpp
)

ADD_LIBRARY(
        sender STATIC

        src/sender/sender.src.cpp
        src/sender/sender.src.hpp
)

ADD_LIBRARY(
        logger STATIC

        src/logger.src.cpp
        src/logger.src.hpp
)

ADD_LIBRARY(
        parsers STATIC

        # The mime parser
        src/parsers/mime-parser.src.hpp
        src/parsers/mime-parser.src.cpp
)

ADD_LIBRARY(
        ossl STATIC

        src/ossl/sha1.src.cpp
        src/ossl/sha256.src.cpp
        src/ossl/ossl.src.hpp
        src/ossl/rsa-sha256.src.cpp
)

ADD_LIBRARY(
        types STATIC

        src/types/mime.src.hpp
)

ADD_LIBRARY(
        debug STATIC

        src/debug/timer.src.hpp
)

ADD_LIBRARY(
        ds_api

        src/ds-api/server.src.hpp
        src/ds-api/server.src.cpp
)

SET_TARGET_PROPERTIES(types PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(debug PROPERTIES LINKER_LANGUAGE CXX)

# The libraries array
SET(libs ds_api server sender parsers models logger lib_db ossl debug)

# ----
# The executable, and linking
# ----

# Creates the main executable
ADD_EXECUTABLE(
        ${PROJECT_NAME}

        main.cpp
        mongoose/mongoose.o
)

# Links the libraries
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${binaries} ${libs} -lresolv -lssl -lcrypto -lpthread -lcassandra)