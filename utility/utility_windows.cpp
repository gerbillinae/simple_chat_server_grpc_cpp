#include "include/utility/utility.h"
#include <windows.h>
#include <iostream>
#include <string>

std::string GetExecutableDir() {
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    std::string::size_type pos = std::string(path).find_last_of("\\/");
    return std::string(path).substr(0, pos);
}