#include <iostream>
#include <fstream>

#include "src/server/server.src.hpp"
#include "src/email.src.hpp"

int main() {
    //Runs the server
   server::run(25);
   // Closes with code 0
   return 0;
}
