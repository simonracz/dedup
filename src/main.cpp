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

tuple<vector<fs::path>, unordered_set<string>> preprocess_dirs(int argc, char* argv[]) {
    auto p = fs::canonical(fs::path{argv[1]});
    unordered_set<string> seen{p.c_str()};
    vector<fs::path> dirs{p}; // stack
    for (int i = 2; i < argc; ++i) {
        fs::path p{argv[i]};
        p = fs::canonical(p);
        if (seen.count(p.c_str()) == 0) {
            seen.insert(p.c_str());
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

    ifstream ifs(file, ios::binary);
    vector<char> bytes(size);
    ifs.read(bytes.data(), size);
    string contents{bytes.data(), size};
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
    unordered_set<string> seen_dirs = get<1>(t); // canonical paths of visited dirs
    unordered_map<string, vector<fs::path>> visited_files; // size + hash_of_content -> [canonical path]
    vector<fs::path> empty_files;

    while (!dirs_to_visit.empty()) {
        fs::path d = dirs_to_visit.back();
        dirs_to_visit.pop_back();
        for (auto it = fs::directory_iterator(d); it != fs::directory_iterator(); ++it) {
            if (fs::is_directory(*it)) {
                fs::path new_dir = fs::canonical(*it);
                if (seen_dirs.count(new_dir.c_str()) == 0) {
                    dirs_to_visit.push_back(new_dir);
                    seen_dirs.insert(new_dir.c_str());
                }
                continue;
            }
            if (fs::is_regular_file(*it)) {
                process_file(fs::canonical(*it), visited_files, empty_files);
                continue;
            }
        }
    }

    cout << "Empty file(s): " << empty_files.size() << endl;
    for (auto& f : empty_files) {
        cout << f << endl;
    }
    cout << endl;

    // TODO
    cout << "Duplicates: " << visited_files.size() << endl;
    for (auto& i : visited_files) {
        cout << "hash: " << i.first << endl;
        for (auto& f : i.second) {
            cout << f << endl;
        }
        cout << "-\n";
    }
    cout << endl;

    return 0;
}