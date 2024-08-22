#include "include/utility/utility.h"
#include <unistd.h>
#include <limits.h>
#include <iostream>
#include <string>

std::string GetExecutableDir() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string path = std::string(result, (count > 0) ? count : 0);
    return path.substr(0, path.find_last_of("/"));
}