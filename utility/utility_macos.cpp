#include "include/utility/utility.h"
#include <mach-o/dyld.h>
#include <limits.h>
#include <iostream>
#include <string>

std::string GetExecutableDir() {
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        std::string fullPath(path);
        return fullPath.substr(0, fullPath.find_last_of("/"));
    } else {
        return std::string();
    }
}