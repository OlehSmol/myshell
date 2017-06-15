#include <iostream>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <dirent.h>

struct Options{
    bool recursive = false;
    bool force = false;
    std::vector<std::string> files;
    int error = 0;
    std::string error_message;
};

bool help_arg(int argc, char *argv[]){
    for(int i=1; i<argc; i++){
        if(argv[i] == "-h" || argv[i] == "--help" ){
            return true;
        }
    }
    return false;
}

struct Options parse_option(int argc, char *argv[]){
    Options opt;
    for(int i=1; i<argc; i++) {
        std::string arg = argv[i];
        if (arg[0] != '-') {
            opt.files.push_back(std::string(argv[i]));
            continue;
        } else{
            for(int i=1; i<arg.size(); i++){
                if(arg[i] == 'f'){
                    opt.force = true;
                } else if(arg[i] == 'r'){
                    opt.recursive = true;
                } else{
                    opt.error = -1;
                    opt.error_message = std::string("invalid option '") + arg + "'\n";
                    return opt;
                }
            }
        }
    }
    return opt;
}

int recursive(std::string file, std::string path = ""){
    chdir(file.c_str());

    DIR *dir;
    struct dirent *entry;
    std::vector<struct dirent*> entries;

    dir = opendir(".");

    while ( (entry = readdir(dir)) != NULL) {
        if(std::string(entry->d_name) == "." || std::string(entry->d_name) == ".."){
            continue;
        }
        if(entry->d_type == 4){
            recursive(std::string(entry->d_name), path + "/" + file);
        }
        if(remove(entry->d_name) == -1){
            std::cout<< "rm: cannot remove '" + path + "/"
                        + file + "/" + std::string(entry->d_name) + "'\n";
        }
    };

    closedir(dir);

    chdir("..");
    return 0;
}

int main(int argc, char *argv[]) {
    if(help_arg(argc, argv)){
        std::cout<< "Usage: rm [OPTION]... [FILE]...\n";
        std::cout<< "Remove (unlink) the FILE(s).\n";
        std::cout<< "f - force;     r - recursive.\n";
    }

    if(argc == 1){
        std::cout << "rm: missing operand\n";
        return 0;
    }

    struct Options opt = parse_option(argc, argv);

    if(opt.error == -1){
        std::cout<<opt.error_message << std::endl;
        return 0;
    }

    for(int i=0; i<opt.files.size(); i++){
        std::string file = opt.files[i];
        std::cout << file <<"\n";
        struct stat st;

        if(stat(file.c_str(), &st) == -1){
            std::cout<< "rm: cannot remove '" + file + "': No such file or directory\n";
            continue;
        }

        if(!opt.force){
            std::string choice;
            std::cout<< "Are you sure to delete '" + file + "'? ('y'): ";
            std::getline(std::cin, choice);
            if(choice != "y")
                continue;
        }

        if(opt.recursive == true){
            if(S_ISDIR(st.st_mode)){
                recursive(file);
            }
            if(remove(file.c_str()) == -1){
                std::cout<< "rm: cannot remove '/" + file + "'\n";
            }
        } else {
            if(S_ISDIR(st.st_mode)){
                std::cout<< "rm: cannot remove '" + file + "': Is a directory\n";
                continue;
            }
            if(remove(file.c_str()) == -1){
                std::cout<< "rm: cannot remove '" + file + "'\n";
            }
        }
    }

    return 0;
}