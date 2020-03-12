#include <iostream>

#include "src/server/server.src.hpp"
#include "src/email.src.hpp"

int main() {
//    server::run(25);
    std::vector<models::EmailAddress> addrl;
    models::parsers::parseAddressList("Luke Rieff<luke.rieff@gmail.com>, Sem Rieff   <sem.rieff@gmail.com>,   <asd@gmail.com>, asd123@gmail.com", addrl);

    for (const auto& a : addrl)
    {
        std::cout << a.e_Address << ":" << a.e_Name << std::endl;
    }

    return 0;
}
