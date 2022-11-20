#include <iostream>

namespace ljf {
struct InitMsg {
    InitMsg(const char *msg) { std::cout << msg << std::endl; }
};

struct DoneMsg {
    const char *const msg;

    // DoneMsg(const char * msg) : msg_(msg) {}
    ~DoneMsg() {
        try {
            std::cout << msg << std::endl;
        } catch (...) {
            // nop
        }
    }
};

} // namespace ljf
