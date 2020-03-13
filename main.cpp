#include <iostream>
#include <fstream>

#include "src/server/server.src.hpp"
#include "src/email.src.hpp"

int main() {
//    server::run(25);
    std::fstream f("../temp.txt");
    std::string line;

    std::string content( (std::istreambuf_iterator<char>(f)),
                         (std::istreambuf_iterator<char>()));

    models::Email target;
    models::parsers::parseMime(content, target);

    return 0;
}
