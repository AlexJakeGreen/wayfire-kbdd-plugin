#include <map>
#include <xkbcommon/xkbcommon.h>
#include <wayfire/plugin.hpp>
#include <wayfire/output.hpp>
#include <wayfire/util/log.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/nonstd/wlroots-full.hpp>


class kbdd_plugin : public wf::plugin_interface_t
{

    std::map<int, int> views;

    int prev_view_id = -1;

    const int default_layout_id = 0;


    int get_layout(int view_id) {
        if (views.find(view_id) == views.end()) {
            return default_layout_id;
        }
        return views.at(view_id);
    }


    void put_layout(int view_id, int layout_id) {
        views[view_id] = layout_id;
    }


    wf::signal_callback_t on_focus_changed = [=] (wf::signal_data_t *data)
    {
        wlr_seat *seat = wf::get_core().get_current_seat();
        wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
        if (!keyboard) {
            return;
        }

        if (prev_view_id) {
            int prev_layout_id = xkb_state_serialize_layout(keyboard->xkb_state,
                                                            XKB_STATE_LAYOUT_LOCKED);
            put_layout(prev_view_id, prev_layout_id);
        }

        wayfire_view view = get_signaled_view(data);
        int view_id = view->get_id();
        prev_view_id = view_id;

        int layout_id = get_layout(view_id);

        wlr_keyboard_notify_modifiers(keyboard,
                                      keyboard->modifiers.depressed,
                                      keyboard->modifiers.latched,
                                      keyboard->modifiers.locked,
                                      layout_id);
    };


public:

    void init() override
    {
        LOGI("kbdd plugin init");
        output->connect_signal("view-focused", &on_focus_changed);
    }

    void fini() override
    {
        output->disconnect_signal("view-focused", &on_focus_changed);
    }

};

DECLARE_WAYFIRE_PLUGIN(kbdd_plugin)
