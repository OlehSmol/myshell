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

std::string name_from_path(std::string path){
    int pos = path.find_last_of("/");
    if(pos == std::string::npos)
        return path;

    return path.substr(pos + 1, path.size() - pos);
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
    if(opt.files.size() == 0){
        std::cout<<"cp: missing file operand\n";
        return 0;
    } else if (opt.files.size() == 1){
        std::cout<<"cp: missing destination file operand after\n";
        return 0;
    } else if(opt.files.size() == 2){
        struct stat st_in;
        if(stat(opt.files[0].c_str(), &st_in) == -1){
            std::cout<< "cp: cannot copy '" + opt.files[0] + "': No such file or directory\n";
            return 0;
        }

        if(S_ISDIR(st_in.st_mode)){
            if(opt.recursive == true){
                //recursive(file);
            } else{
                std::cout<< "cp: omitting directory '/" + opt.files[0] + "'\n";
            }
        } else {
            int fd_w;
            struct stat st_out;
            if(stat(opt.files[1].c_str(), &st_out) == -1){
                fd_w = open(opt.files[1].c_str(), O_TRUNC|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
                if(fd_w == -1){
                    std::cout<< "cp: cannot copy to '" + opt.files[1] + "': No such file or directory\n";
                    return -1;
                }
            } else if(S_ISDIR(st_out.st_mode)){
                std::string name = name_from_path(opt.files[0]);
                std::string cp_path = opt.files[1] + "/" + name;
                if(stat(cp_path.c_str(), &st_out) != -1){
                    std::string choice;
                    std::cout<< "Are you sure to overwrite '" + opt.files[1] + "'? ('y'): ";
                    std::getline(std::cin, choice);
                    if(choice != "y"){
                        return 0;
                    }
                }
                fd_w = open(cp_path.c_str(), O_TRUNC|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

            } else{
                if(opt.force == false){
                    std::string choice;
                    std::cout<< "Are you sure to overwrite '" + opt.files[1] + "'? ('y'): ";
                    std::getline(std::cin, choice);
                    if(choice != "y"){
                        return 0;
                    }
                }
                fd_w = open(opt.files[1].c_str(), O_TRUNC|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
            }
            int fd_r = open(opt.files[0].c_str(), O_RDONLY);
            if(fd_r == -1){
                std::cout<< "cp: can`t open file for reading\n";
                return -1;
            }
            unsigned char buffer[1000];
            int sz;
            while(sz = read(fd_r, buffer, 1000)){
                write(fd_w, buffer, sz);
            }
            close(fd_r);
            close(fd_w);
        }
    } else {
        struct stat st_out;
        if(stat(opt.files[opt.files.size()-1].c_str(), &st_out) == -1){
            std::cout<<"cp: cannot copy to '" + opt.files[opt.files.size()-1] + "': No such file or directory\n";
            return -1;
        } else if(!S_ISDIR(st_out.st_mode)){
            std::cout<<"cp: target '" + opt.files[opt.files.size()-1] + "' is not a directory\n";
            return -1;
        } else{
            for(int i=0; i<opt.files.size()-1; i++){
                struct stat st_in;
                if(stat(opt.files[i].c_str(), &st_in) == -1){
                    std::cout<< "cp: cannot copy '" + opt.files[i] + "': No such file or directory\n";
                    return 0;
                }
                if(S_ISDIR(st_in.st_mode)){
                    if(opt.recursive == true){
                        //recursive(file);
                    } else{
                        std::cout<< "cp: omitting directory '/" + opt.files[i] + "'\n";
                    }
                } else {
                    int fd_r = open(opt.files[i].c_str(), O_RDONLY);
                    if(fd_r == -1){
                        std::cout<<  "cp: cannot copy '" + opt.files[i] + "': No such file or directory\n";
                        continue;
                    }
                    std::string name = name_from_path(opt.files[i]);
                    std::string cp_path = opt.files[opt.files.size()-1] + "/" + name;

                    int fd_w = open(cp_path.c_str(), O_TRUNC|O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

                    if(stat(cp_path.c_str(), &st_out) != -1){
                        if(opt.force == false) {
                            std::string choice;
                            std::cout << "Are you sure to overwrite '" + opt.files[i] + "'? ('y'): ";
                            std::getline(std::cin, choice);
                            if (choice != "y") {
                                continue;
                            }
                        }
                    }

                    unsigned char buffer[1000];
                    int sz;
                    while(sz = read(fd_r, buffer, 1000)){
                        write(fd_w, buffer, sz);
                    }
                    close(fd_r);
                    close(fd_w);
                }
            }
        }
    }
    return 0;
}