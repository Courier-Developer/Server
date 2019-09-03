#ifndef FEVERRPC_UTILS_H_
#define FEVERRPC_UTILS_H_

#include "msgpack.hpp"
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <iostream>

struct Login {
    std::string username;
    std::string password;
    std::string ip;
    MSGPACK_DEFINE(username, password, ip);
    friend std::ostream& operator<<(std::ostream &os, Login &l) {
        std::cout << "[" << l.username << ',' << l.password << ',' << l.ip <<"]\n";
        return os;
    }
};

std::vector<char> read_file(std::string file_name);
void save_file(std::string file_name, std::vector<char> &data);
// int login(Login);

#endif // FEVERRPC_UT<< data<< data<< data<< data<< data<< dataILS_H_