#include "interpreter.h"
#include "builtin.h"
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <algorithm>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1

Interpreter::Interpreter(std::string execpath){
    _execpath = execpath;
}

void Interpreter::clean(){
    _background = false;
    _args.clear();
    _cmd = "";
    _stdout = "";
    _stdin = "";
    _stderr = "";
    _pipe = "";
}

int Interpreter::run_external(){
    if(execv(_args[0].c_str(), (char **)vecstr_to_chararr(_args))){ //try same directory
        _args[0] = _execpath + _args[0];
        if(execv(_args[0].c_str(), (char **)vecstr_to_chararr(_args))){ //try directory with externals
            std::cout << _cmd << ": command not found\n";
            exit(-1);
        }
    }
}

int Interpreter::close_std(){
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

std::string Interpreter::chk_pipe(std::string line){
    int pos = line.find('|');
    if(pos == std::string::npos){
        return line;
    }
    _pipe = line.substr(pos+1, line.size()-pos);
    return line.substr(0, pos);
}

void Interpreter::split_args(std::string line){
    line = line.substr(0, line.find('#'));

    int i = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')){
        i++;
    }
    int pos = i;

    for(i; i<line.size(); i++){
        if(line[i] == ' ' || line[i] == '\t'){
            _args.push_back(line.substr(pos, i-pos));
            while (i < line.size() && (line[i] == ' ' || line[i] == '\t')){
                i++;
            }
            pos = i;
        }
    }

    if(pos < line.size()){
        _args.push_back(line.substr(pos, i-pos));
    }
}

void Interpreter::check_background(){
    if(_args.size() > 1 && _args[_args.size()-1] == "&") {
        _background = true;
        _args.pop_back();
    }
}

std::string Interpreter::subst_var(std::string arg){
    int pos = arg.find('$');
    if(pos == std::string::npos) {
        return arg;
    }
    return arg.substr(0, pos) + _vars[arg.substr(pos+1, arg.size()-pos)];
}

void Interpreter::check_vars(){
    while(!_args.empty()){
        std::string arg = _args[0];
        int pos = arg.find('=');
        if(pos == std::string::npos) {
            return;
        }
        arg = subst_var(arg);
        _vars[arg.substr(0, pos)] = arg.substr(pos+1, arg.size()-pos-1);
        _args.erase(_args.begin());
    }
}

void Interpreter::substitution_vars(){
    for(int i=0; i<_args.size(); i++){
        _args[i] = subst_var(_args[i]);
    }
}

void Interpreter::chk_redirect_output(){
    for (std::vector<std::string>::iterator it=_args.begin()+1; it!=_args.end();)
    {
        if(*it == ">"){
            _args.erase(it);
            if(it!=_args.end()){
                _stdout = *it;
                _args.erase(it);
            }
        } else if(*it == "2>"){
            _args.erase(it);
            if(it!=_args.end()){
                _stderr = *it;
                _args.erase(it);
            }
        } else if(*it == "2>&1" && _stdout != ""){
            _stderr = _stdout;
            _args.erase(it);
        }
        else {
            ++it;
        }
    }
}

void  Interpreter::chk_redirect_input(){
    for (std::vector<std::string>::iterator it=_args.begin(); it!=_args.end();)
    {
        if(*it == "<"){
            _args.erase(it);
            if(it!=_args.end()){
                _stdin = *it;
            }
        } else {
            ++it;
        }
    }
}

void Interpreter::process_args(std::string line) {
    split_args(chk_pipe(line));
    check_background();
    check_vars();
    substitution_vars();
    chk_redirect_input();
    chk_redirect_output();
    if(_args.empty())
        _cmd = "";
    else
        _cmd = _args[0];
}



int Interpreter::execute_line(std::string line, bool ispipe, int in_fd){
    clean();
    process_args(line);

    if(_cmd == ""){
        return 0;
    } else if(_cmd == "cd"){
        my_cd(_args);
        return 0;
    } else if(_cmd == "exit"){
        exit(0);
    }
    int fd[2];

    if(_pipe != ""){
        pipe(fd);
    }
    int pid = fork();
    int status;

    if(pid < 0){
        perror("Fork failed\n");
        return -1;
    } else if(pid == 0) {
        if(_background){
            close_std();
        }
        if(_pipe == ""){
            close(fd[0]);
            dup2(in_fd, STDIN_FILENO);
        } else {
            close(fd[0]);
            dup2(in_fd, STDIN_FILENO);
            dup2(fd[1], STDOUT_FILENO);
        }
        if(_stdin != ""){
            int fd_r = open(_stdin.c_str(), O_RDONLY);
            if(fd_r == -1){
                perror("Can`t open file for writing\n");
                exit(-1);
            }
            dup2(STDIN_FILENO, fd_r);
        }
        if(_stdout != ""){
            int fd_w = open(_stdout.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
            if(fd_w == -1){
                perror("Can`t open file for reading\n");
                exit(-1);
            }
            dup2(fd_w, STDOUT_FILENO);
        }
        if(_stderr != ""){
            int fd_w = open(_stderr.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
            if(fd_w == -1){
                perror("Can`t open file for writing\n");
                exit(-1);
            }
            dup2(fd_w, STDERR_FILENO);
        }

        if(_cmd == "echo"){
            my_echo(_args);
        } else if(_cmd == "pwd"){
            my_pwd(_args);
        } else {
            run_external();
        }
        exit(0);
    } else if(!_background){
        if (_pipe != ""){
            close(fd[1]);
            close(in_fd);
        }
        waitpid(pid, &status, WUNTRACED);
        if(_pipe != ""){
            execute_line(_pipe, true, fd[0]);
        }
    } else {
        std::cout << "[] " << pid << std::endl;
    }
    return 0;
}