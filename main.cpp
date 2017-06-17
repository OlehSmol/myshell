#include <iostream>
#include <string>
#include <vector>
#include "builtin.h"
#include "interpreter.h"
#include <sys/wait.h>
#include <map>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    Interpreter interpreter(getexepath(argv[0]));

    if(argc == 1){ // run console
        while(true){
            std::string line;
            std::cout<< colored_output(curr_dir(), GREEN_OUT) << "$ ";
            std::getline(std::cin, line);

            interpreter.execute_line(line);
        }
    } else if(argc == 2){ // run script
        int fd_r = open(argv[1], O_RDONLY);
        if(fd_r == -1){
            perror("Can`t open file for reading\n");
            return 0;
        }

        std::string line = "";
        unsigned char buffer[1000];
        int sz;

        while(sz = read(fd_r, buffer, 1000)){
            for(int i=0; i<sz; i++) {
                if(buffer[i] == '\n'){
                    if(line.empty()){
                        continue;
                    }
                    interpreter.execute_line(line);
                    line = "";
                } else{
                    line += buffer[i];
                }
            }
        }
    }
    return 0;
}