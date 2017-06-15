#include <iostream>
#include <string>

bool help_arg(int argc, char *argv[]){
    for(int i=1; i<argc; i++){
        if(std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help"){
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    if(help_arg(argc, argv)){
        std::cout<<"Usage: grep [OPTION]... PATTERN [FILE]...\n";
        std::cout<<"Search for PATTERN in each FILE or standard input..\n";
        return 0;
    }

    std::string line;
    while(std::getline(std::cin, line)){
        if(line.find(argv[1]) != std::string::npos){
            std::cout<<line<<std::endl;
        }
    }

    return 0;
}