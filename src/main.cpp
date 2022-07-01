#include <iostream>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

[[noreturn]] void print_usage_and_exit() {
    cerr << "Usage: dedup directory(s)...\n";
    cerr << "  Prints duplicated files in the given directories.\n";
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage_and_exit();
    }
    for (int i = 1; i < argc; ++i) {
        fs::path p{argv[i]};
        if (!fs::is_directory(p)) {
            cerr << argv[i] << " is not a directory.\n\n";
            print_usage_and_exit();
        }
    }
    return 0;
}