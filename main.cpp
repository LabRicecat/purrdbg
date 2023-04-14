#include "execution.hpp"

int main(int argc, char** argv) {
    if(argc != 2) {
        std::cout << 
        "- Purrdbg, the nyasm debugger\n"
        "Usage: purrdbg [file.nyasm]\n";
    }
    else run_debugger(std::string(argv[1]));
}