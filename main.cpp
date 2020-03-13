#include <iostream>

#include "src/server/server.src.hpp"
#include "src/email.src.hpp"

int main() {
    server::run(25);
    return 0;
}
