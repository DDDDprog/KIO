/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/platform.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

#if defined(__linux__)
#include <sys/statvfs.h>
#endif

namespace kio {

std::string PlatformInfo::get_cpu_model() {
#if defined(__linux__)
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    while (std::getline(file, line)) {
        if (line.compare(0, 10, "model name") == 0) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                return line.substr(pos + 2);
            }
        }
    }
#endif
    return "Unknown CPU";
}

long long PlatformInfo::get_total_memory() {
#if defined(__linux__)
    std::ifstream file("/proc/meminfo");
    std::string line;
    while (std::getline(file, line)) {
        if (line.compare(0, 8, "MemTotal") == 0) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::stringstream ss(line.substr(pos + 1));
                long long kb;
                ss >> kb;
                return kb;
            }
        }
    }
#endif
    return -1;
}

long long PlatformInfo::get_root_disk_space() {
#if defined(__linux__)
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        return (long long)stat.f_bsize * stat.f_bavail / 1024;
    }
#endif
    return -1;
}

} // namespace kio
