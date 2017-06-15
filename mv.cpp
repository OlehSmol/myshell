#include <iostream>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <unistd.h>

bool help_arg(int argc, char *argv[]){
    for(int i=1; i<argc; i++){
        if(argv[i] == "-h" || argv[i] == "--help" ){
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    if(help_arg(argc, argv)){
        std::cout<<"help\n";
    }

    struct stat st;
    char* path = argv[1];

    if (stat(path, &st) != 0)
    {
        if (mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR) != 0){
            return -1;
        }
    }
    else if (!S_ISDIR(st.st_mode))
    {
        return -1;
    }

    return 0;
}