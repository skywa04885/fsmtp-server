#pragma once

/**
 * The server configuration
 */
#define GE_DOMAIN "fannst.nl"
#define GE_CASSANDRA_CONTACT_POINTS "192.168.132.133"

/**
 * Define DEBUG in development, will give an advanced log
 */
#define DEBUG

/**
 * Define GUI, when using an Linux Desktop, and want to see an GUI
 */
#define GUI

/**
 * Error messages, used widely
 */
#define PREP_ERROR(a, b)                                                            \
    std::cout << "\033[31m[ERROR]: \033[0m" << __FILE__ << "@" << __LINE__ << ": "  \
    << a << ": " << b << std::endl;

// Will disable some single-line code
#ifdef DEBUG
#define DEBUG_ONLY(a) a;
#else
#define DEBUG_ONLY(a)
#endif

#ifdef GUI
#define GUI_ONLY(a) a;
#else
#define GUI_ONLY(a)
#endif