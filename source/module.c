#include "module.h"
#include "_internal.h"

#define implement_class(name, plugin) \
    ((struct cfb_object_s *)CLASS_IMPL_FN_N(name)(plugin))
static struct cfb_class_helper_s **g_cfb_class_helper;
static struct cfb_plugin_db_s **g_cfb_plugin_db;
static struct cfb_plugin_s **g_cfb_sys_plugin;
static int g_cfb_has_init;

static int on_binding(cfb_plugin_getter_t aPluginGetter, char aState)
{
    static struct cfb_plugin_s **external_plugin;
    aPluginGetter("cfabric@0.0.1", &external_plugin);
    return external_plugin != g_cfb_sys_plugin;
}

static void cfb_init()
{
    if (g_cfb_has_init)
        return;
    g_cfb_has_init = 1;

    //initialise system plugin MUST always be called first
    g_cfb_class_helper = implement_class(cfb_class_helper_s, NULL)->on_new(NULL);
    g_cfb_sys_plugin = implement_class(cfb_plugin_s, NULL)->on_new(NULL);

    //create system objects
    g_cfb_plugin_db = implement_class(cfb_plugin_db_s, g_cfb_sys_plugin)->on_new(NULL);
    (*g_cfb_plugin_db)->publish_plugin(g_cfb_plugin_db, "cfabric@0.0.1", g_cfb_sys_plugin, on_binding);
}
struct cfb_class_helper_s **cfb_class_helper()
{
    cfb_init();
    return g_cfb_class_helper;
}

struct cfb_plugin_s **_cfb_new_plugin()
{
    cfb_init();
    return implement_class(cfb_plugin_s, NULL);
}

struct cfb_plugin_db_s **cfb_plugin_db()
{
    cfb_init();
    return g_cfb_plugin_db;
}