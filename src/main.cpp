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
        xcb_window_t      window;
    } _;

    static auto disconnect() noexcept {
        xcb_disconnect(_.connection);
    }

    static auto destroy_window() noexcept {
        xcb_destroy_window(_.connection, _.window);
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

    // connection here is pretty much required so that connect which sets screen
    // is run before this function.
    static auto screen_of(xcb_connection_t*) noexcept {
        return _.screen;
    }

    struct monitor {
        int16_t  x, y;
        uint16_t w, h;
    };

    static auto primary_output(xcb_connection_t* connection,
                              xcb_screen_t* screen) noexcept {
        auto randr = xcb_get_extension_data(connection, &xcb_randr_id);
        must(randr && randr->present, "%s", "randr is not present");

        auto primary = xcb_randr_get_output_primary_reply(
            connection,
            xcb_randr_get_output_primary(connection, screen->root),
            nullptr
        );
        must(primary, "%s\n", "can't get primary output");

        auto info = xcb_randr_get_output_info_reply(
            connection,
            xcb_randr_get_output_info(connection,
                                      primary->output,
                                      XCB_CURRENT_TIME),
            nullptr
        );
        free(primary);
        must(info, "%s\n", "can't get primary output info");
        must(info->crtc != XCB_NONE, "%s\n",
             "primary output not attached to crtc");
        must(info->connection != XCB_RANDR_CONNECTION_DISCONNECTED, "%s\n",
             "primary output disconnected");

        auto crtc = xcb_randr_get_crtc_info_reply(
            connection,
            xcb_randr_get_crtc_info(connection, info->crtc, XCB_CURRENT_TIME),
            nullptr
        );
        free(info);
        must(crtc, "%s\n", "can't get crtc info");

        auto ret = monitor { crtc->x, crtc->y, crtc->width, crtc->height };
        free(crtc);
        return ret;
    }

    static auto create_window(xcb_connection_t* connection,
                              xcb_screen_t* screen,
                              monitor output) noexcept {
        _.window = xcb_generate_id(connection);
        xcb_create_window(
            connection,
            XCB_COPY_FROM_PARENT,
            _.window,
            screen->root,
            output.x, output.y, output.w,
            12, 0,                              // height, border width
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen->root_visual,
            XCB_CW_BACK_PIXEL        |
            XCB_CW_OVERRIDE_REDIRECT |
            XCB_CW_EVENT_MASK        |
            XCB_CW_COLORMAP,
            (uint32_t []) {
                screen->black_pixel,            // background
                true,                           // wm not to mess with this
                XCB_EVENT_MASK_EXPOSURE,
                XCB_COPY_FROM_PARENT
            }
        );
        std::atexit(destroy_window);

        return _.window;
    }

public:
    static auto run() noexcept {
        auto connection = connect();
        auto screen     = screen_of(connection);
        auto primary    = primary_output(connection, screen);
        auto window     = create_window(connection, screen, primary);
    }
};
decltype(barebar::_) barebar::_;

} // anonymous namespace

auto main() -> int {
    barebar::run();
}
