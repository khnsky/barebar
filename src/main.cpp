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

class barebar {
    // unfortunately we need to use global variables in order to do cleanup at
    // exit.  we can however hide them inside a class and initialize and access
    // them using member functions.
    static struct {
        xcb_connection_t* connection;
        xcb_screen_t*     screen;
    } _;

    static auto disconnect() noexcept {
        xcb_disconnect(_.connection);
    }

    // TODO should there be check so that this can be only run once?
    static auto connect(char const* display = nullptr) noexcept {
        auto n = 0;
        _.connection = xcb_connect(display, &n);

        if (xcb_connection_has_error(_.connection))
            die("[%s:%d] xcb_connect failed\n", __FILE__, __LINE__);

        std::atexit(disconnect);

        auto r = xcb_setup_roots_iterator(xcb_get_setup(_.connection));
        for (auto i = 0; i != n; ++i)
            xcb_screen_next(&r);

        _.screen = r.data;

        return _.connection;
    }

public:
    static auto run() noexcept {
        auto connection = connect();
        (void)connection;
    }
};
decltype(barebar::_) barebar::_;

} // anonymous namespace

auto main() -> int {
    barebar::run();
}
