#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>

struct Options{
    bool recursive = false;
    bool invert = false;
    bool ignore = false;
    std::string pattern = "";
    std::vector<std::string> files;
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

struct Options parse_option(int argc, char *argv[]){
    Options opt;
    for(int i=1; i<argc; i++) {
        std::string arg = argv[i];
        if (arg[0] != '-') {
            if(opt.pattern == "")
                opt.pattern = std::string(argv[i]);
            else
                opt.files.push_back(std::string(argv[i]));
            continue;
        } else{
            for(int i=1; i<arg.size(); i++){
                if(arg[i] == 'v'){
                    opt.invert = true;
                } else if(arg[i] == 'i'){
                    opt.ignore = true;
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

int compare(std::string line, Options opt){
    std::string ln(line);
    std::string pt(opt.pattern);
    if (opt.ignore) {
        std::transform(ln.begin(), ln.end(), ln.begin(), tolower);
        std::transform(pt.begin(), pt.end(), pt.begin(), tolower);
    }
    if (ln.find(pt) != std::string::npos) {
        std::cout << line << std::endl;
    } else if (opt.invert) {
        std::cout << line << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (help_arg(argc, argv)) {
        std::cout << "Usage: grep [OPTION]... PATTERN [FILE]...\n";
        std::cout << "Search for PATTERN in each FILE or standard input..\n";
        return 0;
    }

    struct Options opt = parse_option(argc, argv);

    if (opt.error == -1) {
        std::cout << opt.error_message << std::endl;
        return 0;
    }

    if(opt.pattern == ""){
        std::cout << "Usage: grep [OPTION]... PATTERN [FILE]...\n";
        return -1;
    }

    std::string line;
    if (opt.files.size() == 0){
        while (std::getline(std::cin, line)) {
            compare(line, opt);
        }
    } else{
        for(int i=0; i<opt.files.size(); i++){
            int fd = open(opt.files[i].c_str(), O_RDONLY);

            if(fd == -1){
                std::cout << "grep: " + opt.files[i] + ": No such file or directory";
                return 0;
            }
            close(fd);
        }
        for(int i=0; i<opt.files.size(); i++){
            int fd = open(opt.files[i].c_str(), O_RDONLY);

            char buff[1000];
            int sz;
            std::string line = "";
            while(sz = read(fd, &buff, 1000)){
                for(int i=0; i<sz; i++){
                    if(buff[i] == '\n'){
                        compare(line, opt);
                        line = "";
                    } else {
                        line += buff[i];
                    }
                }
            }
            if(line != ""){
                compare(line, opt);
            }
            close(fd);
        }
    }
    return 0;
}