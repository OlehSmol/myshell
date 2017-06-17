#ifndef MYSHELL_INTERPRETER_H
#define MYSHELL_INTERPRETER_H

#include <map>
#include <string>
#include <vector>
#include <unistd.h>

class Interpreter {
    private:
        std::map <std::string, std::string> _vars;
        std::string _execpath;
        std::vector<std::string> _args;
        std::string _cmd;
        std::string _stdout = "";
        std::string _stdin = "";
        std::string _stderr = "";
        std::string _pipe = "";
        bool _background;
        void clean();
        int close_std();
        std::string chk_pipe(std::string line);
        void split_args(std::string line);
        void check_background();
        std::string subst_var(std::string arg);
        void check_vars();
        void substitution_vars();
        void chk_redirect_output();
        void chk_redirect_input();
        void process_args(std::string line);
        int run_external();
    public:
        Interpreter(std::string execpath);
        int execute_line(std::string line, bool ispipe = false, int in_fd = STDIN_FILENO);
};


#endif