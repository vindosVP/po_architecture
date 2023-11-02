#pragma once
#ifndef HELPER_H
#define HELPER_H

#include <string>
#include "Poco/Base64Decoder.h"
#include <istream>
#include <ostream>

inline bool get_identity(const std::string identity, std::string &login, std::string &password)
{
    std::istringstream istr(identity);
    std::ostringstream ostr;
    Poco::Base64Decoder b64in(istr);
    copy(std::istreambuf_iterator<char>(b64in),
        std::istreambuf_iterator<char>(),
        std::ostreambuf_iterator<char>(ostr));
    std::string decoded = ostr.str();

    size_t pos = decoded.find(':');
    login = decoded.substr(0, pos);
    password = decoded.substr(pos + 1);
    return true;
}


inline bool hasSubstr(const std::string &str, const std::string &substr)
{
    if (str.size() < substr.size())
        return false;
    for (size_t i = 0; i <= str.size() - substr.size(); ++i)
    {
        bool ok{true};
        for (size_t j = 0; ok && (j < substr.size()); ++j)
            ok = (str[i + j] == substr[j]);
        if (ok)
            return true;
    }
    return false;
}

inline long get_hash(const std::string& str) {
    long p = 53;
    long m = 61566613;

    long roll = 0;
    for (auto i = str.crbegin(); i < str.crend(); ++i) {
        roll = (roll * p + int(*i)) % m;
    }

    return roll;
}

#endif