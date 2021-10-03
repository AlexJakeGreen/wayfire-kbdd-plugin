#include <stdlib.h>
#include <xkbcommon/xkbcommon.h>
#include <wayfire/plugin.hpp>
#include <wayfire/output.hpp>
#include <wayfire/util/log.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/nonstd/wlroots-full.hpp>

struct layout {
    int view_id;
    int layout_id;
};

int prev_view_id = -1;
struct layout *layouts;
int layouts_size;

struct layout *find_layout(int view_id) {
    for (int i = 0; i < layouts_size; ++i) {
        if (layouts[i].view_id == view_id) {
            return &layouts[i];
        }
    }
    return NULL;
}

int get_layout(int view_id) {
    struct layout *l = find_layout(view_id);
    return l ? l->layout_id : -1;
}

void put_layout(int view_id, int layout_id) {
    struct layout *existing = find_layout(view_id);
    if (existing) {
        if (layout_id == -1) {
            existing->view_id = -1;
        } else {
            existing->layout_id = layout_id;
        }
        return;
    }

    if (layout_id == -1) {
        return;
    }

    for (int i=0; i < layouts_size; ++i) {
        struct layout *l = &layouts[i];
        if (l->view_id == -1) {
            l->view_id = view_id;
            l->layout_id = layout_id;
            return;
        }
    }

    // realloc
    int old_size = layouts_size;
    layouts_size += 8;
    layouts = (layout *)realloc(layouts, layouts_size * sizeof(struct layout));
    for (int i = old_size; i < layouts_size; ++i) {
        struct layout *l = &layouts[i];
        if (i == old_size) {
            l->view_id = view_id;
            l->layout_id = layout_id;
        } else {
            l->view_id = -1;
        }
    }
}

class kbdd_plugin : public wf::plugin_interface_t
{
    wf::signal_callback_t on_focus_changed = [=] (wf::signal_data_t *data)
    {
        wlr_seat *seat = wf::get_core().get_current_seat();
        wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
        if (!keyboard) {
            LOGI("==> keyboard is null");
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

        LOGI("==> on_focus_changed");
        LOGI("==> view: ", view_id);

        int layout_id = get_layout(view_id);
        if (layout_id < 0) {

            layout_id = xkb_state_serialize_layout(keyboard->xkb_state,
                                                   XKB_STATE_LAYOUT_LOCKED);

            LOGI("==> stashed layout ID: ", layout_id);
        }

        wlr_keyboard_notify_modifiers(keyboard,
                                      keyboard->modifiers.depressed,
                                      keyboard->modifiers.latched,
                                      keyboard->modifiers.locked,
                                      layout_id);
    };

public:
    void init() override
    {
        output->connect_signal("view-focused", &on_focus_changed);
    }

    void fini() override
    {
        output->disconnect_signal("view-focused", &on_focus_changed);
    }

};

DECLARE_WAYFIRE_PLUGIN(kbdd_plugin)
