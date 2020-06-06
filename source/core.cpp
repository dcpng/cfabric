#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
//#include <windows.h>
#include "cfabric.h"

/**
 * c++ STL libraries
 */
#include <map>
#include <vector>
#include <forward_list>
#include <string>
#include <iterator>

struct _cfb_class_hnd_info_s
{
    /*
    0 -> undefined
    1 -> concrete class
    2 -> interface class
     */
    int kind;

    /*
    the vector index value where to reference to
    the class methods are stored
    */
    int value;
};

/*
* This type is used in the mapping class's string name to its unique handle local
* to the running process
*/
typedef std::map<std::string, struct _cfb_class_hnd_info_s> _cfb_map_of_class_hnds;
typedef std::vector<struct cfb_obj_lifecycle_iface_s *> _cfb_array_of_class_lifecycles;

struct _cfb_plugin_cntxt_s
{
    /*
    [!!****MUST BE FIRST, DON"T ADD FIELDS ON TOP (Crash casting!)***!!]
    ptr to the system-services provided by the plugin such as
    - set_class, get_class_hnd, etc
    */
    cfb_plugin_t *Methods;

    /*
    1: if the plugin is sealed and set_class is disabled. Define is set to 0 (zero)
    */
    int is_sealed;

    /*
    Indexes the canonical classnames of _cfb_array_of_class_lifecycles class_lifecycles. Note that, the double
    indirection is done so that to elimimate the need for the classes to be load 
    some prescribed sequence.
     */
    _cfb_map_of_class_hnds map_of_class_hnds;

    /**
     * points callback function to initialise the plugin.
     */
    cfb_init_plugin_cb_t init_plugin_cb;
    /**
     * points the process-wide array of class life cycles.
     */
    _cfb_array_of_class_lifecycles *processwide_classe_lifecycles;
};

/*
This allows the storage of multiple versions of pluging
*/
typedef std::map<int /*version number*/, struct _cfb_plugin_cntxt_s *> _cfb_map_of_plugin_cntxt;

/*
* This type is used in the mapping class's string name to its unique handle local
* to the running process
*/
typedef std::map<std::string, _cfb_map_of_plugin_cntxt *> _cfb_plugin_map;

static _cfb_array_of_class_lifecycles processwide_classe_lifecycles;

/**
 * sets the class's cfb_obj_lifecycle_iface_s call backs for managing the instantiation
 * of the class.
 * ARG:
 *      aClass: reference to the life cycle hooks to be called `get_obj_by_class_hnd`
 *      name: the class type string name for the object
 * return
 *      0: On success
 *     -1: ERROR: conflicting with an existing registration
 *     -2: ERROR: plugin is sealed
 */
static int set_class(void *aThis, cfb_obj_lifecycle_iface_s *aClass, char *aName)
{

    struct _cfb_class_hnd_info_s Entry;
    struct _cfb_plugin_cntxt_s *vThis = (struct _cfb_plugin_cntxt_s *)aThis;

    if (vThis->is_sealed)
    {
        return -2;
    }

    if (vThis->map_of_class_hnds.find(aName) != vThis->map_of_class_hnds.end())
    {
        Entry = vThis->map_of_class_hnds[aName];
        if (Entry.kind != 0)
        {
            return -1;
        }
        Entry.kind = aClass == NULL ? 2 : 1;
        (*(vThis->processwide_classe_lifecycles))[Entry.value] = aClass;
    }
    else
    {
        Entry.kind = 1;
        Entry.value = vThis->processwide_classe_lifecycles->size();
        vThis->processwide_classe_lifecycles->push_back(aClass);
    }
    vThis->map_of_class_hnds[aName] = Entry;
    return 0;
}

static cfb_class_hnd_t get_class_hnd(void *aThis, char *aName)
{
    _cfb_plugin_cntxt_s *vThis = (_cfb_plugin_cntxt_s *)aThis;
    if (vThis->map_of_class_hnds.find(aName) != vThis->map_of_class_hnds.end())
    {
        return vThis->map_of_class_hnds[aName].value;
    }
    int vOffset = vThis->processwide_classe_lifecycles->size();
    vThis->map_of_class_hnds[aName] = (struct _cfb_class_hnd_info_s){.kind = 0, .value = vOffset};
    vThis->processwide_classe_lifecycles->push_back(NULL);
    return vOffset;
}

static void *get_obj_by_class_hnd(void *aThis, cfb_class_hnd_t aClassHandle)
{
    struct _cfb_plugin_cntxt_s *vThis = (struct _cfb_plugin_cntxt_s *)aThis;
    if (0 <= aClassHandle && aClassHandle < vThis->processwide_classe_lifecycles->size())
    {
        struct cfb_obj_lifecycle_iface_s *vClassDef = (cfb_obj_lifecycle_iface_s *)(*(vThis->processwide_classe_lifecycles))[aClassHandle];
        if (vClassDef != NULL)
        {
            return vClassDef->on_new((cfb_plugin_t **)vThis);
        }
    }
    return NULL;
}

static void seal(void *aThis)
{
    struct _cfb_plugin_cntxt_s *vThis = (struct _cfb_plugin_cntxt_s *)aThis;
    vThis->is_sealed = 1;
}

static cfb_plugin_t _impl = {
    .set_class = set_class,
    .get_class_hnd = get_class_hnd,
    .get_obj_by_class_hnd = get_obj_by_class_hnd,
    .seal = seal};

static _cfb_plugin_map _cfb_map_of_plugin_names;

/**
 * This function converts a version str to
 */
static int _version_str_to_num(char **aStr, int level = 0)
{
    char vRuleStr[6];
    int vlen = 0, vlsh;
    switch (level)
    {
    case 0:
        vlsh = 24;
        break;
    case 1:
        vlsh;
        vlsh = 12;
        break;
    case 2:
        vlsh = 0;
        break;
    default:
        return 0;
    }

    for (char v = **aStr; isdigit(v); v = *(++*aStr))
    {
        vRuleStr[vlen++] = v;
    }

    if (vlen == 0)
    {
        return -1;
    }
    vRuleStr[vlen] = 0;
    return (atoi(vRuleStr) << vlsh) + (**aStr != '.' ? 0 : ({++*aStr;
        _version_str_to_num(aStr, level + 1); }));
}

/*
* This function compares a version value "x.x.x" with a filter rules aRuleStr
* arg:
*   aRuleStr : the filter rule (please refer to header file)
*   aVersion : version number
*   
*/
static int _cmp_version(char *aRuleStr, int aVersion)
{
    int vMin = _version_str_to_num(&aRuleStr, 0), vMax, vModVer = aVersion;

    if (vMin == -1)
    { // if no version was defined then just return 0
        return 1;
    }
    else
    {
        switch (*(aRuleStr++))
        {
        case '+':
            return (vModVer - vMin) >= 0;
        case '-':
            return (vMin <= vModVer) && (vModVer <= _version_str_to_num(&aRuleStr, 0));
        case '<':
            return (vMin <= vModVer) && (vModVer < _version_str_to_num(&aRuleStr, 0));
        default:
            return vMin == vModVer;
        }
    }
}

static int _tokenise_name_verstr(char *ptr, char **vVerStr)
{
    for (char i = *ptr; i != 0; i = *(ptr++))
    {
        if (i == '@')
        {
            *vVerStr = ptr;
            break;
        }
    }

    if (*(--ptr) == '@')
    {
        *ptr = 0;
    }
    else
    {
        // ERROR:Ensure all plugins contains a version number
        return -1;
    }
    return 0;
}

int cfb_define_plugin(char *aStmtStr, cfb_plugin_t ***aResult, cfb_init_plugin_cb_t aCb)
{
    char plugin_name[256], *ptr = plugin_name, *vVerStr;

    //prevent buffer overflow
    if (strlen(aStmtStr) > 255)
        return -1;

    //seperate the module name part from the
    //version part
    strcpy(plugin_name, aStmtStr);
    if (int i = _tokenise_name_verstr(plugin_name, &vVerStr))
    {
        return i;
    }

    int vVerNum = _version_str_to_num(&vVerStr);

    cfb_plugin_t **vResult;

    _cfb_map_of_plugin_cntxt *map_of_plugin_cntxt;
    if (_cfb_map_of_plugin_names.find(plugin_name) != _cfb_map_of_plugin_names.end())
    {
        map_of_plugin_cntxt = (_cfb_map_of_plugin_cntxt *)_cfb_map_of_plugin_names[plugin_name];
    }
    else
    {
        _cfb_map_of_plugin_names[plugin_name] = map_of_plugin_cntxt = new _cfb_map_of_plugin_cntxt();
    }

    if (map_of_plugin_cntxt->find(vVerNum) != map_of_plugin_cntxt->end())
    {
        return -2;
    }

    struct _cfb_plugin_cntxt_s *new_plugin = new _cfb_plugin_cntxt_s();
    new_plugin->Methods = (cfb_plugin_t *)&_impl;
    new_plugin->processwide_classe_lifecycles = &processwide_classe_lifecycles;
    new_plugin->init_plugin_cb = aCb;
    (*map_of_plugin_cntxt)[vVerNum] = new_plugin;
    *aResult = (cfb_plugin_t **)&new_plugin->Methods;

    return 0;
}

static int _cfb_plugin_getter_imp(char *aQryStr, cfb_plugin_t ***aResult)
{
    char plugin_name[256], *ptr = plugin_name, *vFilterStr;

    //prevent buffer overflow
    if (strlen(aQryStr) > 255)
        return -1;

    //seperate the module name part from the
    //version part
    strcpy(plugin_name, aQryStr);
    if (int i = _tokenise_name_verstr(plugin_name, &vFilterStr))
    {
        return i;
    }

    if (_cfb_map_of_plugin_names.find(plugin_name) != _cfb_map_of_plugin_names.end())
    {
        _cfb_map_of_plugin_cntxt *map_of_plugin_cntxt;
        map_of_plugin_cntxt = (_cfb_map_of_plugin_cntxt *)_cfb_map_of_plugin_names[plugin_name];
        for (auto rit = map_of_plugin_cntxt->rbegin(); rit != map_of_plugin_cntxt->rend(); ++rit)
        {
            if (_cmp_version(vFilterStr, rit->first))
            {
                *aResult = (cfb_plugin_t **)&rit->second->Methods;
                return 0;
            }
        }
        return -2; // ERROR: Can't find compatible version
    }
    return -3; // ERROR: Cant't find plugin name
}

int cfb_init_all_plugins()
{
    for (auto rit = _cfb_map_of_plugin_names.begin(); rit != _cfb_map_of_plugin_names.end(); ++rit)
    {
        for (auto rit2 = rit->second->begin(); rit2 != rit->second->end(); ++rit2)
        {
            rit2->second->init_plugin_cb(_cfb_plugin_getter_imp, 'l');
        }
    }
}