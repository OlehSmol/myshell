#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <algorithm>

std::string curr_dir(){
    return get_current_dir_name();
}

std::string getexepath(char *argv)
{
    std::string path = std::string(argv);
    path = path.substr(0, path.find_last_of("/")+1);
    if(path[0] == '/'){
        return path;
    } else {
        return std::string(curr_dir() + '/' + path);
    }
}

std::string colored_output(std::string line, const std::string color){
    return color + line + "\033[0m";
}

const char** vecstr_to_chararr(std::vector<std::string> args){
    const char** argv = new const char* [args.size()]+1;
    for(int i=0; i< args.size(); i++){
        argv[i] = args[i].c_str();
    }
    argv[args.size()] = (char *)0;
    return argv;
}

bool help_arg(std::vector<std::string> args){
    for(int i=1; i<args.size(); i++){
        if(args[i] == "-h" || args[i] == "--help" ){
            return true;
        }
    }
    return false;
}

int my_pwd(std::vector<std::string> args){
    if(help_arg(args)){
        std::cout << "This command show current directory\n";
        return 0;
    }
    std::cout<< curr_dir() << std::endl;
    return 0;
}

int my_cd(std::vector<std::string> args){
    if(help_arg(args)){
        std::cout << "This command change current directory\n";
        return 0;
    }

    if(args.size() == 1){
        chdir("/");
        return 0;
    }

    if(chdir(args[1].c_str()) == -1){
        perror((args[0] + ": " + args[1]).c_str());
    }

    return 0;
}

int my_echo(std::vector<std::string> args){
    for(int i=1; i<args.size(); i++){
        std::cout << args[i] << " ";
    }

    std::cout<<std::endl;
}


