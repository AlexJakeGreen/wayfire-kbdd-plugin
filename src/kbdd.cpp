#include <xkbcommon/xkbcommon.h>
#include <wayfire/plugin.hpp>
#include <wayfire/signal-definitions.hpp>
#include "wayfire/seat.hpp"
#include <wayfire/nonstd/wlroots-full.hpp>
#include "xkb_utils.hpp"

#define UNUSED(x) (void)(x)


int prev_layout_id = 0;
char prev_layout_name[128];

void write_file(int layout_id, const char *layout_name) {
    char layout_code[3] = "??";
    get_layout_code(layout_code, layout_name);

    std::string filename = wf::option_wrapper_t<std::string>("kbdd/export_layout_filename");
    FILE *f = fopen(filename.c_str(), "w");

    char buffer[128];
    sprintf(buffer, "{\"id\": %d, \"name\": \"%s\", \"code\": \"\%s\"}\n", layout_id, layout_name, layout_code);

    fprintf(f, buffer);
    fclose(f);
}


struct view_layout_id : public wf::custom_data_t
{
    int layout_id;

    view_layout_id(int id) {
        this->layout_id = id;
    };

    virtual ~view_layout_id() = default;
};


class kbdd_plugin : public wf::plugin_interface_t
{

    uint prev_view_id = -1;
    const int default_layout_id = 0;
    const char *view_layout_id_key = "keyboard-layout-id";


    int get_view_layout(wayfire_view view) {
        if (view && view->has_data(view_layout_id_key)) {
            return view->get_data<view_layout_id>(view_layout_id_key)->layout_id;
        }

        return default_layout_id;
    }


    void restore_view_layout(wayfire_view view, wlr_keyboard *keyboard) {
        int layout_id = get_view_layout(view);

        wlr_keyboard_notify_modifiers(keyboard,
                                      keyboard->modifiers.depressed,
                                      keyboard->modifiers.latched,
                                      keyboard->modifiers.locked,
                                      layout_id);
    };


    void save_view_layout(uint view_id, wlr_keyboard *keyboard) {
        if (view_id > 0) {
            int layout_id = xkb_state_serialize_layout(keyboard->xkb_state,
                                                       XKB_STATE_LAYOUT_LOCKED);

            for (auto v : wf::get_core().get_all_views()) {
                if (v->get_id() == view_id) {
                    v->store_data(std::make_unique<view_layout_id>(layout_id),
                                  view_layout_id_key);
                    break;
                }
            }
        }
    };


    wf::signal::connection_t<wf::keyboard_focus_changed_signal> keyboard_focus_changed = [=] (wf::keyboard_focus_changed_signal *signal) {
        wf::scene::node_ptr focused_node = signal->new_focus;
        wayfire_view view = wf::node_to_view(focused_node);

        if (!view)
            return;

        wlr_seat *seat = wf::get_core().get_current_seat();
        wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
        if (!keyboard)
            return;

        save_view_layout(prev_view_id, keyboard);

        int view_id = view->get_id();
        prev_view_id = view_id;

        restore_view_layout(view, keyboard);

        update_layout_indicator();
    };


    void update_layout_indicator() {
        wlr_seat *seat = wf::get_core().get_current_seat();
        wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

        if (!keyboard) {
            return;
        }

        int layout_id = xkb_state_serialize_layout(keyboard->xkb_state, XKB_STATE_LAYOUT_EFFECTIVE);

        xkb_keymap *keymap = keyboard->keymap;
        const char *layout_name = xkb_keymap_layout_get_name(keymap, layout_id);

        if (prev_layout_id == layout_id && strcmp(prev_layout_name, layout_name) == 0) {
            // avoid extra disk IO
            return;
        }
        write_file(layout_id, layout_name);
        prev_layout_id = layout_id;
        strcpy(prev_layout_name, layout_name);
    }


    wf::signal::connection_t<wf::post_input_event_signal<wlr_keyboard_key_event>> key_press =
        [this] (wf::post_input_event_signal<wlr_keyboard_key_event> *ev) {

            UNUSED(ev);

            update_layout_indicator();

        };


public:
    void init() override
    {
        load_xkb_registry();
        wf::get_core().connect(&keyboard_focus_changed);
        wf::get_core().connect(&key_press);
        update_layout_indicator();
    }

    void fini() override {
        unload_xkb_registry();
        wf::get_core().disconnect(&keyboard_focus_changed);
        wf::get_core().disconnect(&key_press);
    }
};

DECLARE_WAYFIRE_PLUGIN(kbdd_plugin)
