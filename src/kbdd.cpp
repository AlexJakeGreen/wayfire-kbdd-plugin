#include <xkbcommon/xkbcommon.h>
#include <wayfire/plugin.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/nonstd/wlroots-full.hpp>


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
    };


public:
    void init() override
    {
        wf::get_core().connect(&keyboard_focus_changed);
    }

    void fini() override {
        wf::get_core().disconnect(&keyboard_focus_changed);
    }
};

DECLARE_WAYFIRE_PLUGIN(kbdd_plugin)
