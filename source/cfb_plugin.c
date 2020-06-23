#include "_internal.h"

static UT_array *g_class_lc_array;
static UT_icd class_lc_icd = {sizeof(struct cfb_object_s *), NULL, NULL, NULL};

struct class2Hnd_t
{
    char *class_name;
    int id;
    UT_hash_handle hh;
};

struct cfb_plugin_sFields
{
    /*
    Indexes the canonical classnames of _cfb_array_of_class_lifecycles class_lifecycles. Note that, the double
    indirection is done so that to elimimate the need for the classes to be load 
    some prescribed sequence.
     */
    struct class2Hnd_t *class2hnds;
    /*
    1: if the plugin is sealed and set_class is disabled. Define is set to 0 (zero)
    */
    int is_sealed;
};

BEGIN_CLASS_INTERFACES(cfb_plugin_s)

END_CLASS

static void *on_new(void *argValues)
{
    if (!g_class_lc_array)
    {
        utarray_new(g_class_lc_array, &class_lc_icd);
    }
    __auto_type vTest = sizeof(struct cfb_plugin_sFields);
    return (*cfb_class_helper())->alloc_obj(ClassInfo(cfb_plugin_s));
}

static void on_del(void **aThis)
{
}

/**
 * sets the class's cfb_object_s call backs for managing the instantiation
 * of the class.
 * ARG:
 *      aClass: reference to the life cycle hooks to be called `get_obj_by_class_hnd`
 *      name: the class type string name for the object
 * return
 *      0: On success
 *     -1: ERROR: conflicting with an existing registration
 *     -2: ERROR: plugin is sealed
 */
static int decl_class(void *aThis, struct cfb_object_s *aClass, char *aName)
{

    struct cfb_plugin_sFields *vData = (*cfb_class_helper())->get_obj_data(aThis);

    if (vData->is_sealed)
    {
        return -2;
    }

    struct class2Hnd_t *s;
    HASH_FIND_STR(vData->class2hnds, aName, s);

    if (s)
    {
        return -1;
    }
    s = (typeof(s))calloc(1, sizeof *s);
    s->class_name = aName;
    s->id = utarray_len(g_class_lc_array);
    HASH_ADD_KEYPTR(hh, vData->class2hnds, s->class_name, strlen(s->class_name), s);
    utarray_push_back(g_class_lc_array, &aClass);
    return 0;
}

static int decl_interface(void *aThis, char *aClassName)
{
    static int iface_counter = -2; // statice var is initialized only once!
    struct cfb_plugin_sFields *vData = (*cfb_class_helper())->get_obj_data(aThis);

    if (vData->is_sealed)
    {
        return -2;
    }

    struct class2Hnd_t *s;
    HASH_FIND_STR(vData->class2hnds, aClassName, s);

    if (s)
    {
        return -1;
    }
    else
    {
        s = (typeof(s))calloc(1, sizeof *s);
        s->class_name = aClassName;
        s->id = iface_counter--;
        HASH_ADD_KEYPTR(hh, vData->class2hnds, s->class_name, strlen(s->class_name), s);
    }

    return 0;
}

static cfb_class_hnd_t get_class_hnd(void *aThis, char *aName)
{
    struct cfb_plugin_sFields *vData = (*cfb_class_helper())->get_obj_data(aThis);
    struct class2Hnd_t *s;
    HASH_FIND_STR(vData->class2hnds, aName, s);
    if (!s)
    {
        return -1;
    }
    return s->id;
}

static void *get_obj_by_class_hnd(void *aThis, cfb_class_hnd_t aClassHandle, void *aArg)
{
    struct cfb_plugin_sFields *vData = (*cfb_class_helper())->get_obj_data(aThis);
    if (aClassHandle < 0 || aClassHandle >= utarray_len(g_class_lc_array))
    {
        return NULL;
    }
    __auto_type vClassDef = (struct cfb_object_s *)utarray_eltptr(g_class_lc_array, aClassHandle);
    return vClassDef->on_new(aArg);
}

static void seal(void *aThis)
{
    struct cfb_plugin_sFields *vData = (*cfb_class_helper())->get_obj_data(aThis);
    vData->is_sealed = 1;
}

BEGIN_CLASS_IMPLEMENTATION(cfb_plugin_s)
{
    MAP_METHOD(decl_class);
    MAP_METHOD(decl_interface);
    MAP_METHOD(get_class_hnd);
    MAP_METHOD(get_obj_by_class_hnd);
    MAP_METHOD(seal);
}
END_CLASS_IMPLEMENTATION