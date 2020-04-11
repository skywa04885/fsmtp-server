#pragma once

#define DEBUG

// Global error messages
#define PREP_ERROR(a, b)                                                            \
    std::cout << "\033[31m[ERROR]: \033[0m" << __FILE__ << "@" << __LINE__ << ": "  \
    << a << ": " << b << std::endl;

// Enables debug only code
#ifdef DEBUG
#define DEBUG_ONLY(a) a;
#else
#define DEBUG_ONLY(a)
#endif