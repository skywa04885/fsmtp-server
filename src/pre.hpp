/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#pragma once

// ----
// The server configuration
// ----

#define GE_DOMAIN "fannst.nl"
#define GE_CASSANDRA_CONTACT_POINTS "192.168.132.133"

// -----
// Define DEBUG in development, will give an advanced log
// ----
#define DEBUG

// ----
// Preprocessing error messages
// ----
#define PREP_ERROR(a, b)                                                            \
    std::cout << "\033[31m[ERROR]: \033[0m" << __FILE__ << "@" << __LINE__ << ": "  \
    << a << ": " << b << std::endl;

// Will disable some single-line code
#ifdef DEBUG
#define DEBUG_ONLY(a) a;
#else
#define DEBUG_ONLY(a)
#endif

// ----
// Allocation helpers
// ----

// Adds one byte to otherwise invalid memory for string
#define ALLOC_CAS_STRING(a, b) a + b + 1
#define ALLOCATE_NULL_TERMINATION(a) a + 1

// ----
// Simple type defintions
// ----

#define BYTE char

// ----
// Simple char definitions
// ----

#define CRLF "\r\n"
