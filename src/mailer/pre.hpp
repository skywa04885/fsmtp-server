#define FSMT_DEBUG

#define FSMT_PREP_ERROR(a, b)                                                       \
    std::cout << "\033[31m[ERROR]: \033[0m" << __FILE__ << "@" << __LINE__ << ": "  \
    << a << ": " << b << std::endl;

#define FSMTP_PREP_DEBUG_PRINT(a, b)                                                \
    std::cout << "\033[34m[DEBUG]: \033[0m" << __FILE__ << "@" << __LINE__ << ": "  \
    << a << ": " << b << std::endl;

#ifdef FSMT_DEBUG
#define FSMT_DEBUG_ONLY(a) a;
#else
#define FSMT_DEBUG_ONLY(a)
#endif