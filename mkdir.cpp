#include <iostream>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <string.h>

struct Options{
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
            opt.error = -1;
            opt.error_message = std::string("invalid option '") + arg + "'\n";
            return opt;
        }
    }
    return opt;
}

int main(int argc, char *argv[]) {
    if(help_arg(argc, argv)){
        std::cout<<"Usage: mkdir [OPTION]... DIRECTORY...\n";
        std::cout<<"Create the DIRECTORY(ies), if they do not already exist.\n";
        return 0;
    }

    if(argc == 1){
        std::cout << "mkdir: missing operand\n";
        return 0;
    }

    struct Options opt = parse_option(argc, argv);

    if(opt.error == -1){
        std::cout<<opt.error_message << std::endl;
        return 0;
    }

    for(int i=0; i<opt.files.size(); i++){
        const char* path = opt.files[i].c_str();
        if (mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR) == -1){
            if(errno == EACCES){
                std::cout<< std::string("mkdir: cannot create directory '") +
                        path + "': Permission denied\n";
            } else if(errno == EEXIST){
                std::cout<< std::string("mkdir: cannot create directory '") +
                            path + "': File exists\n";
            } else if(errno == EEXIST){
                std::cout<< std::string("mkdir: cannot create directory '") +
                            path + "': File exists\n";
            } else if(errno == ENOENT){
                std::cout<< std::string("mkdir: cannot create directory '") +
                            path + "': No such file or directory\n";
            } else{
                std::cout<< std::string("mkdir: cannot create directory '") +
                            path + "'\n";
            }
            continue;
        }
    }


    return 0;
}