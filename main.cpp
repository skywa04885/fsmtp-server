#include <iostream>
#include <fstream>

#include "src/server/server.src.hpp"
#include "src/email.src.hpp"

int main() {
//    server::run(25);
    std::fstream f("../tt.txt");

    if (!f.is_open())
    {
        return -1;
    }

    std::string line;

    std::string content( (std::istreambuf_iterator<char>(f)),
                         (std::istreambuf_iterator<char>()));

    models::Email target;
    models::parsers::parseMime(content, target);
    std::cout << target << std::endl;

    f.close();

    return 0;
}
