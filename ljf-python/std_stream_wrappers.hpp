
#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

namespace ljf::python::detail {
class StdIStreamWrapper {
    std::istream &istream_;
    std::string prompt_;

public:
    /*implicit*/ StdIStreamWrapper(std::istream &is) : istream_(is) {}

    std::string getline() {
        std::string s;
        std::getline(istream_, s);
        prompt(prompt_);
        return s;
    }

    bool eof() { return istream_.eof(); }

    template <typename Str> void prompt(Str &&str) {
        auto out_p = istream_.tie();
        if (!out_p) {
            return;
        }

        auto &out = *out_p;
        out << std::forward<Str>(str);
    }
};

class StdFStreamWrapper {
    std::fstream fstream_;

public:
    /*implicit*/ StdFStreamWrapper(std::fstream &&fs)
        : fstream_(std::move(fs)) {}

    std::string getline() {
        std::string s;
        std::getline(fstream_, s);
        return s;
    }

    bool eof() { return fstream_.eof(); }

    template <typename Str> void prompt(Str &&) {
        // do nothing
    }
};

class StdStringStreamWrapper {
    std::stringstream stream_;

public:
    /*implicit*/ StdStringStreamWrapper(std::stringstream &&fs)
        : stream_(std::move(fs)) {}
    /*implicit*/ StdStringStreamWrapper(std::string s)
        : stream_(std::move(s)) {}

    std::string getline() {
        std::string s;
        std::getline(stream_, s);
        return s;
    }

    bool eof() { return stream_.eof(); }

    template <typename Str> void prompt(Str &&) {
        // do nothing
    }
};
} // namespace ljf::python::detail
