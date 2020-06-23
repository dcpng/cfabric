#pragma once
#include <stdint.h>
#include "module.h"
#include "../lib/include/uthash/utarray.h"
#include "../lib/include/uthash/uthash.h"

typedef int version_t;

#ifdef __cplusplus
extern "C"
{
#endif
    struct cfb_plugin_s **_cfb_new_plugin();
    CLASS_INIT_FUNC(cfb_plugin_db_s);
    CLASS_INIT_FUNC(cfb_class_helper_s);
    CLASS_INIT_FUNC(cfb_plugin_s);
#ifdef __cplusplus
}
#endif