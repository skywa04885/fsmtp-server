/*
 * Project: <SMTP> FSMTP Server by Luke Rieff
 * Author: Luke Rieff
 * Github: https://github.com/skywa04885/fsmtp-server
 * Copyright: Free to use, without modifying
 */

#include "strings.hpp"

/**
 * Creates an new fannst char buffer
 * @return
 */
fcb fcb_new()
{
    // Allocates one byte
    fcb n = reinterpret_cast<fcb>(malloc(1));

    // Adds the null termination
    n[0] = '\0';

    // Returns the string
    return n;
}

/**
 * Creates an new fannst char buffer with an default size
 * @return
 */
fcb fcb_new(const int& size)
{
    // Allocates one byte
    fcb n = reinterpret_cast<fcb>(malloc(size + 1));

    // Adds the null termination
    n[0] = '\0';

    // Returns the string
    return n;
}

/**
 * Creates an new fannst char buffer with default value
 * @return
 */
fcb fcb_new(const char *val)
{
    // Gets the length of the val, plus one for null termination
    std::size_t valLen = strlen(&val[0]) + 1;

    // Allocates the required space
    fcb n = reinterpret_cast<fcb>(malloc(valLen));

    // Copies the value into the fcb
    memcpy(&n[0], &val[0], valLen);

    // Returns the new fcb
    return n;
}

/**
 * Gets an fannst char buffer length
 * @param a
 * @return
 */
std::size_t fcb_strlen(fcb a)
{
    // The result
    std::size_t r = 0;

    // Loop while string not ended
    while (*a != '\0')
    {
        r++;
        a++;
    }

    // Returns the result
    return r;
}

/**
 * Concats two fannst char buffers
 * @param a
 * @param b
 */
void fcb_strcat(fcb a, fcb b)
{
    // Gets the length of a and b
    std::size_t aLen = fcb_strlen(&a[0]);
    std::size_t bLen = fcb_strlen(&b[0]);

    // Copies the memory
    memcpy(&a[aLen], &b[0], bLen);

    // Sets the new null termination char
    a[aLen + bLen] = '\0';
}

/**
 * Concats one fannst char buffer and const char *
 * @param a
 * @param b
 */
void fcb_strcat(fcb a, const char * b)
{
    // Gets the length of a and b
    std::size_t aLen = fcb_strlen(&a[0]);
    std::size_t bLen = strlen(&b[0]);

    // Copies the memory
    memcpy(&a[aLen], &b[0], bLen);

    // Sets the new null termination char
    a[aLen + bLen] = '\0';
}

/**
 * Concats one fannst char buffer and const char *
 * @param a
 * @param b
 */
void fcb_strcat(fcb a, char b)
{
    // Gets the length of a and b
    std::size_t aLen = fcb_strlen(&a[0]);

    // Copies the memory
    memcpy(&a[aLen], &b, 1);

    // Sets the new null termination char
    a[aLen + 1] = '\0';
}

/**
 * Concats one fannst char buffer and int
 * @param a
 * @param b
 */
void fcb_strcat(fcb a, const int& b)
{
    // Creates the buffer
    char buffer[16];
    // Prints the int
    sprintf(&buffer[0], "%d", b);
    // Concats the result
    fcb_strcat(a, buffer);
}

/**
 * Resizes one fannst char buffer
 * @param a
 * @param len
 */
void fcb_resize(fcb *a, const std::size_t& len)
{
    // Resize the buffer
    *a = reinterpret_cast<char *>(realloc(&(*a)[0], len+1));
}

/**
 * Resizes one fannst char buffer
 * @param a
 * @param len
 */
void fcb_resize(fcb *a, const int& len)
{
    // Resize the buffer
    *a = reinterpret_cast<char *>(realloc(&(*a)[0], len+1));
}

/**
 * Finds an char in fannst char buffer
 * @param a
 * @param b
 * @return
 */
std::size_t fcb_findchar(fcb a, char b)
{
    // The result
    std::size_t r = 0;

    // Loop while string not ended
    while (*a != '\0')
    {
        // Checks if it should break
        if (*a == b) break;

        // Increments the indexes
        r++;
        a++;
    }

    // Returns the result
    return r;
}