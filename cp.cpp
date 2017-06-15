#include <iostream>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

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



int main(int argc, char *argv[]) {
    if(help_arg(argc, argv)){
        std::cout<<"help\n";
    }

    if(argc < 3){
        std::cout << "cp: missing operand\n";
        return 0;
    }

    struct Options opt = parse_option(argc, argv);

    if(opt.error == -1){
        std::cout<<opt.error_message << std::endl;
        return 0;
    }

    if(opt.files.size() == 2){
        struct stat st_in;
        if(stat(opt.files[0].c_str(), &st_in) == -1){
            std::cout<< "cp: cannot stat '" + opt.files[0] + "': No such file or directory\n";
            return 0;
        }

        if(S_ISDIR(st_in.st_mode)){
            if(opt.recursive == true){
                //recursive(file);
            } else{
                std::cout<< "cp: omitting directory '/" + opt.files[0] + "'\n";
            }
        } else {
            struct stat st_out;
            if(stat(opt.files[1].c_str(), &st_out) == -1){

            } else if(S_ISDIR(st_out.st_mode)){

            } else{
                if(opt.force == true){
                    std::string choice;
                    std::cout<< "Are you sure to overwrite '" + opt.files[1] + "'? ('y'): ";
                    std::getline(std::cin, choice);
                    if(choice != "y"){
                        return 0;
                    }
                }

            }
        }

    }
    int fd_r = open(argv[1], O_RDONLY);

    return 0;
}