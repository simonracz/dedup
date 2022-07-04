#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sha256.h>

using namespace std;
namespace fs = std::filesystem;

[[noreturn]] void print_usage_and_exit() {
    cerr << "Usage: dedup directory(s)...\n";
    cerr << "  Prints duplicated files in the given directories.\n";
    exit(EXIT_FAILURE);
}

tuple<vector<fs::path>, unordered_set<fs::path>> preprocess_dirs(int argc, char* argv[]) {
    unordered_set<fs::path> seen;
    vector<fs::path> dirs{fs::absolute(fs::path{argv[1]})}; // stack
    for (int i = 2; i < argc; ++i) {
        fs::path p{argv[i]};
        p = fs::absolute(p);
        if (seen.count(p) == 0) {
            seen.insert(p);
            dirs.push_back(p);
        }
    }
    return make_tuple(dirs, seen);
}

void process_file(const fs::path& file, unordered_map<string, vector<fs::path>>& visited_files, vector<fs::path>& empty_files) {
    uintmax_t size = fs::file_size(file);
    if (size == 0) {
        empty_files.push_back(file);
        return;
    }
    // 20 MB
    if (size > 20971520) {
        string key = to_string(size);
        if (visited_files.count(key) == 0) {
            visited_files[key] = vector{file};
        } else {
            visited_files[key].push_back(file);
        }
        return;
    }
    string contents;
    contents.reserve(size);
    ifstream ifs(file, ios::binary);
    ifs.read(contents.data(), size);
    auto h = sha256(contents);
    if (visited_files.count(h) == 0) {
        visited_files[h] = vector{file};
    } else {
        visited_files[h].push_back(file);
    }
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
    auto t = preprocess_dirs(argc, argv);
    vector<fs::path> dirs_to_visit = get<0>(t); // stack
    unordered_set<fs::path> seen_dirs = get<1>(t); // absolute paths of visited dirs
    unordered_map<string, vector<fs::path>> visited_files; // size + hash_of_content -> [absolute path]
    vector<fs::path> empty_files;

    while (!dirs_to_visit.empty()) {
        fs::path d = dirs_to_visit.back();
        dirs_to_visit.pop_back();
        for (auto it = fs::directory_iterator(d); it != fs::directory_iterator(); ++it) {
            if (fs::is_directory(*it)) {
                fs::path new_dir = fs::absolute(*it);
                if (seen_dirs.count(new_dir) == 0) {
                    dirs_to_visit.push_back(new_dir);
                    seen_dirs.insert(new_dir);
                }
                continue;
            }
            if (fs::is_regular_file(*it)) {
                process_file(fs::absolute(*it), visited_files, empty_files);
                continue;
            }
        }
    }


    return 0;
}