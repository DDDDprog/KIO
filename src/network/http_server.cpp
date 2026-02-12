/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/network/http_server.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <map>

namespace kio {

struct ServerInstance {
    int fd;
    int port;
};

static std::vector<ServerInstance> instances;

Value native_server_init(int argCount, Value* args) {
    if (argCount < 1) return doubleToValue(-1);
    int port = (int)args[0].toNumber();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "[HTTP] Socket creation failed" << std::endl;
        return doubleToValue(-1);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "[HTTP] setsockopt failed" << std::endl;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "[HTTP] Bind failed on port " << port << std::endl;
        return doubleToValue(-1);
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "[HTTP] Listen failed" << std::endl;
        return doubleToValue(-1);
    }

    std::cout << "[HTTP] Server initialized on port " << port << " (fd: " << server_fd << ")" << std::endl;
    instances.push_back({server_fd, port});
    return doubleToValue((double)instances.size() - 1);
}

Value native_server_accept(int argCount, Value* args) {
    if (argCount < 1) return NIL_VAL;
    int id = (int)args[0].toNumber();
    if (id < 0 || id >= (int)instances.size()) return NIL_VAL;

    int server_fd = instances[id].fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    
    if (new_socket < 0) {
        std::cerr << "[HTTP] Accept failed" << std::endl;
        return NIL_VAL;
    }

    char buffer[4096] = {0};
    int valread = read(new_socket, buffer, 4096);
    if (valread <= 0) {
        close(new_socket);
        return NIL_VAL;
    }
    std::string request(buffer);

    std::stringstream ss(request);
    std::string method, path, protocol;
    ss >> method >> path >> protocol;

    std::cout << "[HTTP] Connection accepted: " << method << " " << path << std::endl;

    ObjArray* res = new ObjArray();
    res->elements.push_back(doubleToValue((double)new_socket));
    res->elements.push_back(objToValue(new ObjString(method)));
    res->elements.push_back(objToValue(new ObjString(path)));
    res->elements.push_back(objToValue(new ObjString(request))); // Raw request body/headers
    
    return objToValue(res);
}

Value native_server_respond(int argCount, Value* args) {
    if (argCount < 4) return NIL_VAL;
    int client_fd = (int)args[0].toNumber();
    int status = (int)args[1].toNumber();
    std::string type = args[2].toString();
    std::string body = args[3].toString();

    std::stringstream res;
    res << "HTTP/1.1 " << status << (status == 200 ? " OK" : " Error") << "\r\n";
    res << "Content-Type: " << type << "\r\n";
    res << "Content-Length: " << body.length() << "\r\n";
    res << "Connection: close\r\n\r\n";
    res << body;

    std::string response = res.str();
    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);

    std::cout << "[HTTP] Sent response (" << status << ", " << body.length() << " bytes)" << std::endl;
    return TRUE_VAL;
}

} // namespace kio
