#include "_internal.h"

struct version_s
{
    version_t value; /* key */
    struct cfb_plugin_s **plugin;
    cfb_bind_plugin_cb_t bind_plugin_cb;
    UT_hash_handle hh;
};

struct plugin_name_s
{
    char value[32]; /* key */
    int is_sorted;
    struct version_s *version_list;
    UT_hash_handle hh;
};

struct cfb_plugin_db_sFields
{
    struct plugin_name_s *ht
};

BEGIN_CLASS_INTERFACES(cfb_plugin_db_s)

END_CLASS

/**
 * This function converts a version str to a 32 bit version number
 * @param aStr points to the version string
 * @return the version integer
 */
static version_t _version_str_to_num(char **aStr)
{
    version_t _impl(char **aStr, int level)
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
            _impl(aStr, level + 1); }));
    }
    return _impl(aStr, 0);
}

/**
* This function compares a version value "x.x.x" with a filter rules aRuleStr
* @param : the filter rule (please refer to header file)
* @param : version number
* @return 1 if version fits the rules
*/
static int _cmp_version(char *aRuleStr, int aVersion)
{
    version_t vMin = _version_str_to_num(&aRuleStr), vMax, vModVer = aVersion;

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
            return (vMin <= vModVer) && (vModVer <= _version_str_to_num(&aRuleStr));
        case '<':
            return (vMin <= vModVer) && (vModVer < _version_str_to_num(&aRuleStr));
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

static int _cfb_plugin_getter_imp(char *aQryStr, cfb_plugin_t ***aResult)
{
    char buff[256], *ptr = buff, *vFilterStr;
    struct cfb_plugin_db_sFields *vData = (*cfb_class_helper())->get_obj_data(cfb_plugin_db());

    //prevent buffer overflow
    if (strlen(aQryStr) > 255)
        return -1;

    //seperate the module name part from the
    //version part
    strcpy(buff, aQryStr);
    {
        int i = _tokenise_name_verstr(buff, &vFilterStr);
        if (i)
        {
            return i;
        }
    }

    struct plugin_name_s *v_plugin_name;

    if (vData->ht)
        HASH_FIND_STR(vData->ht, buff, v_plugin_name);

    if (!v_plugin_name)
    {
        return -3; // ERROR: Cant't find plugin name
    }

    int sort_function(void *a, void *b)
    {
        return (*(int *)b - *(int *)a);
    }

    if (!v_plugin_name->is_sorted)
    {
        v_plugin_name->is_sorted = 1;
        HASH_SORT(v_plugin_name->version_list, sort_function);
    }

    struct version_s *s;
    for (s = v_plugin_name->version_list; s != NULL; s = (typeof(s))s->hh.next)
    {
        if (_cmp_version(vFilterStr, s->value))
        {
            *aResult = s->plugin;
            return 0;
        }
    }

    return -2; // ERROR: Can't find compatible version
}

static int bind_all_plugins(void **aThis)
{
    struct cfb_plugin_db_sFields *vData = (*cfb_class_helper())->get_obj_data(aThis);
    struct plugin_name_s *pn_current, *pn_temp;
    int error = 0;
    HASH_ITER(hh, vData->ht, pn_current, pn_temp)
    {
        struct version_s *ver_current, *ver_temp;
        HASH_ITER(hh, pn_current->version_list, ver_current, ver_temp)
        {
            error = error | ver_current->bind_plugin_cb(_cfb_plugin_getter_imp, 'i');
        }
    }
    return error;
}

static void *on_new(void **aInput)
{
    __auto_type vResult = (*cfb_class_helper())->alloc_obj(ClassInfo(cfb_plugin_db_s));
    struct cfb_plugin_db_sFields *vData = (*cfb_class_helper())->get_obj_data(vResult);
    return vResult;
}

static void on_del(void **aThis)
{
    /*TODO delete everything*/
}

static int publish_plugin(void **aThis, char *aStmtStr, cfb_plugin_t **aPlugin, cfb_bind_plugin_cb_t aCb)
{
    char buff[256], *ptr = buff, *vVerStr;
    struct cfb_plugin_db_sFields *vData = (*cfb_class_helper())->get_obj_data(aThis);
    //prevent buffer overflow
    if (strlen(aStmtStr) > 255)
        return -1;

    if (aPlugin == NULL)
    {
        return -2;
    }

    //seperate the module name part from the
    //version part
    strcpy(buff, aStmtStr);
    {
        int i = _tokenise_name_verstr(buff, &vVerStr);
        if (i)
        {
            return i;
        }
    }
    int vVerNum = _version_str_to_num(&vVerStr);

    struct plugin_name_s *v_plugin_name;
    HASH_FIND_STR(vData->ht, buff, v_plugin_name);
    if (!v_plugin_name)
    {
        v_plugin_name = (typeof(v_plugin_name))calloc(1, sizeof *v_plugin_name);
        strncpy(v_plugin_name->value, buff, sizeof(v_plugin_name->value) - 1);
        HASH_ADD_STR(vData->ht, value, v_plugin_name);
    }

    struct version_s *vVersion;
    HASH_FIND_INT(v_plugin_name->version_list, vVerNum, vVersion);

    // If the version already exist then return error
    if (vVersion)
    {
        return -1;
    }

    vVersion = (typeof(vVersion))calloc(1, sizeof *vVersion);
    HASH_ADD_INT(v_plugin_name->version_list, value, vVersion);
    vVersion->plugin = aPlugin;
    vVersion->value = vVerNum;
    vVersion->bind_plugin_cb = aCb;
    return 0;
}

cfb_plugin_t **new_plugin(void **aThis)
{
    return _cfb_new_plugin();
}

BEGIN_CLASS_IMPLEMENTATION(cfb_plugin_db_s)
{
    MAP_METHOD(publish_plugin);
    MAP_METHOD(new_plugin);
    MAP_METHOD(bind_all_plugins);
}
END_CLASS_IMPLEMENTATION
