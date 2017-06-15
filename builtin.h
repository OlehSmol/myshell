#ifndef MYSHELL_BUILTIN_H
#define MYSHELL_BUILTIN_H

#endif //MYSHELL_BUILTIN_H

#include <string>
#include <vector>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <sys/wait.h>


const std::string BLACK_OUT = "\033[22;30m";
const std::string RED_OUT = "\033[22;31m";
const std::string GREEN_OUT = "\033[22;32m";
const std::string DEFAULT_OUT = "\033[0m";

std::string getexepath(char *argv);
std::string colored_output(std::string line, const std::string color);

const char** vecstr_to_chararr(std::vector<std::string> args);

std::string curr_dir();
bool help_arg(std::vector<std::string> args);

int my_pwd(std::vector<std::string> args);
int my_cd(std::vector<std::string> args);
int my_echo(std::vector<std::string> args);