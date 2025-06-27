#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <string>
#include "../include/utils.hpp"

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos)
    {
        token = str.substr(start, end - start);
        tokens.push_back(token);
        start = end + 1;
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));

    return tokens;
}

std::string c_strip(char *str)
{
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t'))
        end--;
    *(end + 1) = '\0';
    return str;
}

std::string append_number(const std::string &str, int number)
{
    std::stringstream ss;
    ss << str << number;
    return ss.str();
}