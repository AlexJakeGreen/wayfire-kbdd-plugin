#ifndef KBDD_XKB_UTILS_H_
#define KBDD_XKB_UTILS_H_

#include <xkbcommon/xkbregistry.h>

int load_xkb_registry();
void unload_xkb_registry();
int get_layout_code(char *layout_code, const char * layout_description);

#endif
