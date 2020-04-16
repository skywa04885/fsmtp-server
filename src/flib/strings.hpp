/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include <malloc.h>
#include <cstddef>
#include <cstring>

#include "../pre.hpp"

// ----
// Type definitions
// ----

typedef char * fcb;

// ----
// The string methods
// ----

/**
 * Creates an new fannst char buffer
 * @return
 */
fcb fcb_new();

/**
 * Creates an new fannst char buffer with default value
 * @return
 */
fcb fcb_new(const char *val);

/**
 * Creates an new fannst char buffer with an default size
 * @return
 */
fcb fcb_new(const int& size);

/**
 * Clears an fannst char buffer
 * @param a
 */
#define fcb_free(a) free(a)

/**
 * Gets an fannst char buffer length
 * @param a
 * @return
 */
std::size_t fcb_strlen(fcb a);

/**
 * Concats two fannst char buffers
 * @param a
 * @param b
 */
void fcb_strcat(fcb a, fcb b);

/**
 * Concats one fannst char buffer and const char *
 * @param a
 * @param b
 */
void fcb_strcat(fcb a, const char * b);

/**
 * Concats one fannst char buffer and int
 * @param a
 * @param b
 */
void fcb_strcat(fcb a, const int& b);

/**
 * Concats one fannst char buffer and const char *
 * @param a
 * @param b
 */
void fcb_strcat(fcb a, char b);

/**
 * Resizes one fannst char buffer
 * @param a
 * @param len
 */
void fcb_resize(fcb *a, const std::size_t& len);

/**
 * Resizes one fannst char buffer
 * @param a
 * @param len
 */
void fcb_resize(fcb *a, const int& len);

/**
 * Finds an char in fannst char buffer
 * @param a
 * @param b
 * @return
 */
std::size_t fcb_findchar(fcb a, char b);