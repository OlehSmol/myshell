#include <iostream>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
#include <time.h>

const std::string SYS_DIRS[] = {".", ".."};
const int SYS_DIRS_N = 2;

struct Options{
    bool recursive = false;
    bool all = false;
    bool sort = false;
    bool (*sortparam)(struct dirent*, struct dirent*);
    bool isregexp = false;
    std::vector<std::string> regexp;
    std::vector<std::string> pattern;
    std::vector<std::string> path;
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

bool compare_name(struct dirent* entry1, struct dirent* entry2) {
    std::string name1 = std::string(entry1->d_name);
    std::string name2 = std::string(entry2->d_name);
    std::transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    std::transform(name2.begin(), name2.end(), name2.begin(), ::tolower);
    return  name1 < name2;
}

bool compare_size(struct dirent* entry1, struct dirent* entry2) {
    struct stat st1, st2;
    stat(entry1->d_name, &st1);
    stat(entry2->d_name, &st2);
    return st1.st_size < st2.st_size;
}

bool compare_ext(struct dirent* entry1, struct dirent* entry2) {
    if(entry1->d_type == 4)
        return true;
    if(entry2->d_type == 4)
        return false;

    std::string name1(entry1->d_name);
    int dot1_pos = name1.find('.');
    if(dot1_pos == std::string::npos)
        return true;

    std::string name2(entry2->d_name);
    int dot2_pos = name2.find('.');
    if(dot2_pos == std::string::npos)
        return false;

    return name1.substr(dot1_pos+1, name1.size()-dot1_pos) <
            name2.substr(dot2_pos+1, name2.size()-dot2_pos);
}

bool compare_time(struct dirent* entry1, struct dirent* entry2) {
    struct stat st1, st2;
    stat(entry1->d_name, &st1);
    stat(entry2->d_name, &st2);
    return st1.st_mtim.tv_sec < st2.st_mtim.tv_sec;
}

struct Options parse_option(int argc, char *argv[]){
    Options opt;
    for(int i=1; i<argc; i++) {
        std::string arg = argv[i];
        if (arg[0] != '-') {
            if(std::string(arg).find_first_of("?*[]") != std::string::npos){
                opt.pattern.push_back(arg);
            } else {
                opt.path.push_back(arg);
            }
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
        for(int j=1; j<arg.size(); j++){
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

bool match(const char *first, const char * second) {
    if (*first == '\0' && *second == '\0')
        return true;
    if (*first == '*' && *(first+1) != '\0' && *second == '\0')
        return false;
    if (*first == '?' || *first == *second)
        return match(first+1, second+1);

    if (*first == '[' || *first == *second){
        int i=1;
        while(*(first+i) != ']'){
            if(*(first+i) == '\0') {
                return false;
            }
            i++;
        }
        for(int j=1; j<i; j++){
            if(*(first+j) == *second){
                return match(first+i+1, second+1);
            }
        }
    }

    if (*first == '*')
        return match(first+1, second) || match(first, second+1);

    return false;
}

int dir_ls(std::string path, Options opt, bool pattern = false){
    std::vector<struct dirent*> entries = entries_to_vec();
    if(opt.rever){
        std::reverse(entries.begin(), entries.end());
    }

    if(opt.recursive && entries.size() > 2){
        std::cout << std::endl << path << ":" << std::endl;
    }

    if(opt.sort){
        sort(entries.begin(), entries.end(), opt.sortparam);
    }
    
    for(int i=0; i<entries.size(); i++){
        std::string name(entries[i]->d_name);

        if(name == ".." || name == ".")
            continue;
        if(pattern){
            bool matched = false;
            for(int i=0; i<opt.pattern.size(); i++){
                if(match(opt.pattern[i].c_str(), entries[i]->d_name)){
                    matched = true;
                    break;
                }
            }
            if(!matched)
                continue;
        }

        if(entries[i]->d_type == 4)
            std::cout << "/";
        std::cout << name;

        if(opt.longinfo){
            struct stat st;
            stat(name.c_str(), &st);
            std::cout << "\t" << st.st_size;
            char buff[20];
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtim.tv_sec));
            std::cout << "\t" << buff;
        }
        std::cout << std::endl;
    }

    if(opt.recursive) {
        for (int i = 0; i < entries.size(); i++) {
            std::string name(entries[i]->d_name);

            if(name == ".." || name == ".")
                continue;

            if (entries[i]->d_type == 4) {
                chdir(name.c_str());
                dir_ls(path + "/" + name, opt);
                chdir("..");
            }

        }
    }
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

    if(opt.pattern.size() + opt.path.size() == 0){
        opt.path.push_back("..");
    }

    char* cur_dir = get_current_dir_name();

    for (int i = 0; i < opt.path.size(); ++i) {
        if(chdir(opt.path[i].c_str()) != -1){
            if( opt.path.size() > 1){
                std::cout << "\n" + opt.path[i] << ":\n";
            }
            dir_ls(opt.path[i], opt);
            chdir(cur_dir);
        } else {
            std::cout << "ls: cannot access '" + opt.path[i] + "': No such file or directory";
        }
    }

    if(!opt.pattern.empty())
        dir_ls(".", opt, true);


    return 0;
}