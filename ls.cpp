#include <iostream>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>

const std::string SYS_DIRS[] = {".", ".."};
const int SYS_DIRS_N = 2;

struct Options{
    bool recursive = false;
    bool all = false;
    bool sort = false;
    bool (*sortparam)(struct dirent*, struct dirent*);
    bool isregexp = false;
    std::vector<std::string> regexp;
    bool rever = false;
    bool longinfo = false;
    int error = 0;
    std::string error_message;
};

bool help_arg(int argc, char *argv[]){
    for(int i=1; i<argc; i++){
        if(std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help"){
            return true;
        }
    }
    return false;
}


std::vector<struct dirent*> entries_to_vec(std::string path =""){
    DIR *dir;
    struct dirent *entry;
    std::vector<struct dirent*> entries;

    if(path[0] == '/' ){
        dir = opendir(path.c_str());
    } else {
        dir = opendir((get_current_dir_name() + path).c_str());
    }

    if (!dir) {
        std::cout<< "ls: cannot access: '" << path << "' No such file or directory";
        perror("diropen");
        exit(1);
    };

    while ( (entry = readdir(dir)) != NULL) {
        entries.push_back(entry);
    };

    closedir(dir);

    return entries;
}

bool filter(struct dirent* entry, Options opt){
    if(!opt.all){
        for(int i=0; i<SYS_DIRS_N; i++){
            if(std::string(entry->d_name) == SYS_DIRS[i]){
                return false;
            }
        }
    }
    return true;
}

bool compare_name(struct dirent* entry1, struct dirent* entry2) {
    std::string name1 = std::string(entry1->d_name);
    std::string name2 = std::string(entry2->d_name);
    std::transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    std::transform(name2.begin(), name2.end(), name2.begin(), ::tolower);

    return  name1 < name2;
}

bool compare_size(struct dirent* entry1, struct dirent* entry2) {
    return (int)entry1->d_reclen < (int)entry2->d_reclen;
}

bool compare_ext(struct dirent* entry1, struct dirent* entry2) {
    return entry1->d_reclen < entry2->d_reclen;
}

bool compare_time(struct dirent* entry1, struct dirent* entry2) {
    return entry1->d_name < entry2->d_name;
}

struct Options parse_option(int argc, char *argv[]){
    Options opt;
    for(int i=1; i<argc; i++) {
        std::string arg = argv[i];
        if (arg[0] != '-') {
            opt.isregexp = true;
            opt.regexp.push_back(std::string(argv[i]));
            continue;
        }
        if (arg.substr(0, 7) == "--sort=") {
            if (arg.size() > 8) {
                opt.error = -1;
                opt.error_message = "ls: invalid argument '" + \
                        arg.substr(7, arg.size() - 7) + "' for ‘--sort’";
                return opt;
            }
            char param = arg[7];
            if(param == 'U'){
                opt.sort = false;
            } else {
                opt.sort = true;
                if (param == 'S') {
                    opt.sortparam = compare_size;
                } else if (param == 'X') {
                    opt.sortparam = compare_ext;
                } else if (param == 'N') {
                    opt.sortparam = compare_name;
                } else if (param == 't') {
                    opt.sortparam = compare_time;
                } else {
                    opt.error = -1;
                    opt.error_message = std::string("ls: option '") + param + "' is ambiguous";
                    return opt;
                }
            }
            continue;
        }
        for(int j=0; j<arg.size(); j++){
            if(arg[j] == 'r'){
                opt.rever = true;
            } else if(arg[j] == 'l'){
                opt.longinfo = true;
            } else if(arg[j] == 'R') {
                opt.recursive = true;
            } else {
                opt.error = -1;
                opt.error_message = "ls: option '" + arg + "' is ambiguous";
                return opt;
            }
        }
    }
    return opt;
}

int main(int argc, char *argv[]) {
    if(help_arg(argc, argv)){
        std::cout<<"List information about the FILEs (the current directory by default).\n";
        return 0;
    }

    struct Options opt = parse_option(argc, argv);

    if(opt.error == -1){
        std::cout<<opt.error_message << std::endl;
        return 0;
    }

    std::vector<struct dirent*> entries = entries_to_vec();
    opt.sort = true;
    opt.sortparam = compare_size;
    if(opt.sort == true){
        std::sort(entries.begin(), entries.end(), opt.sortparam);
    }

    if(opt.rever){
        std::reverse(entries.begin(), entries.end());
    }

    for(int i=0; i<entries.size(); i++){
        if(!filter(entries[i], opt))
            continue;
        if(entries[i]->d_type == 4)
            std::cout << "/";
        std::cout << entries[i]->d_name;
        std::cout << std::endl;
    }



    return 0;
}