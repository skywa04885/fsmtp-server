#include "logger.src.hpp"

namespace logger {
    Console::Console(const Level &c_Level, const char *c_Prefix) {
        this->c_Level = c_Level;
        this->c_Prefix = c_Prefix;
    }

    Console::~Console() {
        return;
    }

    void Console::setLevel(const Level &c_Level) {
        this->c_Level = c_Level;
    }
};