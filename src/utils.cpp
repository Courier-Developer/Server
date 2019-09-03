#include <feverrpc/utils.hpp>

int login(Login) { return 1; }

std::vector<char> read_file(std::string file_name) {
    std::ifstream file(file_name, std::ios::binary);
    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> vec;
    vec.reserve(fileSize);

    vec.insert(vec.begin(), std::istream_iterator<char>(file),
               std::istream_iterator<char>());
    file.close();
    return vec;
}

void save_file(std::string file_name, std::vector<char> &data) {
    std::ofstream file(file_name, std::ios::out | std::ios::binary);
    file.write((const char *)&data[0], data.size());
    file.close();
}