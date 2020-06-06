#include "cfabric.h"

struct class_hnds
{
    cfb_class_hnd_t cfb_class_helper_s;
} g_hnds;

static int init_plugin(cfb_plugin_getter_t aPluginGetter, char aState);

void load_cfabric(cfb_define_plugin_t define_plugin)
{
    cfb_plugin_t **vMyCtx;
    cfb_define_plugin("cfabric@1.0.0", &vMyCtx, init_plugin);

    // Put all the class implementation here
    ImplementClass(cfb_class_helper_s, vMyCtx);
}

static int init_plugin(cfb_plugin_getter_t aPluginGetter, char aState)
{
    struct cfb_plugin_s **mod;
    int err;

    /*loads an external model*/
    if (err = aPluginGetter("cfabric@1.0.1", &mod))
    {
        return err;
    }

    /*get the hnd of the class name*/
    g_hnds.cfb_class_helper_s = (*mod)->get_class_hnd(mod, ClassName(cfb_class_helper_s));
    struct cfb_class_helper_s **vObj = (*mod)->get_obj_by_class_hnd(mod, g_hnds.cfb_class_helper_s);

    return 0;
}