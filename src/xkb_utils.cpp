#include <string.h>

#include "xkb_utils.hpp"

#ifndef XKB_RULES_FILE
#define XKB_RULES_FILE "evdev"
#endif

struct rxkb_context *ctx;


int load_xkb_registry() {

    enum rxkb_context_flags flags = RXKB_CONTEXT_NO_FLAGS;

    ctx = rxkb_context_new (flags);

    if (!rxkb_context_parse (ctx, XKB_RULES_FILE)) {
        rxkb_context_unref (ctx);
        return 1;
    }

    return 0;
}


void unload_xkb_registry() {

    rxkb_context_unref (ctx);

}


int get_layout_code(char *layout_code, const char * layout_description) {

    struct rxkb_layout *layout;

    for (layout = rxkb_layout_first (ctx); layout; layout = rxkb_layout_next (layout)) {

        const char *name, *description;
        name = rxkb_layout_get_name (layout);
        description = rxkb_layout_get_description (layout);

        if (0 == strcmp(description, layout_description)) {
            strcpy(layout_code, name);
            return 0;
        }

    }

    return 1;
}
