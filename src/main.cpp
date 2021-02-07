#include <cstdio>
#include <cstdlib>
#include <err.h>
#include <xcb/randr.h>
#include <xcb/xcb.h>

// enclose everything in a anonymous namespace for that sweet internal linkage.
namespace {

template<class... Args>
[[noreturn]] auto die(const char* fmt, Args... args) {
    // I would use <format> but it is not yet implemented in std libs.
    std::fprintf(stderr, fmt, args...);
    exit(EXIT_FAILURE);
}

template<class T, class... Args>
auto must(T&& t, char const* fmt, Args... args) {
    if (!t)
        die(fmt, args...);
    return t;
}

}

auto main() -> int {
}
