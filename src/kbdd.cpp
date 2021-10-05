#include <map>
#include <xkbcommon/xkbcommon.h>
#include <wayfire/plugin.hpp>
#include <wayfire/output.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/nonstd/wlroots-full.hpp>


class kbdd_plugin : public wf::plugin_interface_t
{
    std::map<int, int> views;

    int prev_view_id = -1;

    const int default_layout_id = 0;


    void restore_view_layout(int view_id, wlr_keyboard *keyboard) {
        int layout_id;

        if (views.find(view_id) == views.end()) {
            layout_id = default_layout_id;
        } else {
            layout_id = views.at(view_id);
        }

        wlr_keyboard_notify_modifiers(keyboard,
                                      keyboard->modifiers.depressed,
                                      keyboard->modifiers.latched,
                                      keyboard->modifiers.locked,
                                      layout_id);
    };


    void save_prev_view_layout(wlr_keyboard *keyboard) {
        if (prev_view_id > 0) {
            int prev_layout_id = xkb_state_serialize_layout(keyboard->xkb_state,
                                                            XKB_STATE_LAYOUT_LOCKED);
            views[prev_view_id] = prev_layout_id;
        }
    };


    void delete_view_layout(int view_id) {
        views.erase(view_id);
        if (views.size() == 0) {
            wlr_seat *seat = wf::get_core().get_current_seat();
            wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
            if (!keyboard) {
                return;
            }
            wlr_keyboard_notify_modifiers(keyboard,
                                          keyboard->modifiers.depressed,
                                          keyboard->modifiers.latched,
                                          keyboard->modifiers.locked,
                                          default_layout_id);
        }
    };


    wf::signal_callback_t on_focus_changed = [=] (wf::signal_data_t *data)
    {
        wlr_seat *seat = wf::get_core().get_current_seat();
        wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
        if (!keyboard) {
            return;
        }

        save_prev_view_layout(keyboard);

        wayfire_view view = get_signaled_view(data);
        int view_id = view->get_id();
        prev_view_id = view_id;

        restore_view_layout(view_id, keyboard);
    };


    wf::signal_callback_t on_view_unmapped = [=] (wf::signal_data_t *data) {
        wayfire_view view = get_signaled_view(data);
        int view_id = view->get_id();
        prev_view_id = -1;

        delete_view_layout(view_id);
    };


public:
    void init() override
    {
        output->connect_signal("view-focused", &on_focus_changed);
        output->connect_signal("view-pre-unmapped", &on_view_unmapped);
    }

    void fini() override
    {
        output->disconnect_signal("view-focused", &on_focus_changed);
        output->disconnect_signal("view-pre-unmapped", &on_view_unmapped);
    }
};

DECLARE_WAYFIRE_PLUGIN(kbdd_plugin)
