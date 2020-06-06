#pragma once

#define CFABRIC_MOD_NAME "cfabric"

/*Use this to safely generate the string for the class type identifer
to get handles of other classes. Note the double casting will trigger
the compiler to check if the ident was valid*/
#define ClassName(name) \
    ((char *)&((struct name *)#name)[0])

/**
 * This magic number is used by the class_operator to cast to the default interface
 * of the object
 */
#define IObjectHandler -1

typedef int cfb_class_hnd_t;
typedef struct cfb_plugin_s cfb_plugin_t;

/*This interface is to be implemented by all classes
that can create objects (of itself) and/or delete those
objects*/
struct cfb_obj_lifecycle_iface_s
{
    /*called when object destruction*/
    void (*on_del)(void ** /*this*/);

    /*called when object instanciation*/
    void *(*on_new)(cfb_plugin_t **aCtx);
};

/*plugin loader is a collection to load*/
struct cfb_plugin_s
{
    /*
    This method maps the cfb_obj_lifecycle_iface_s for the class, which is
    defined by the string name of the class
    args:
        aClass -> Address to the Implementation of the Concrete Class. 
        aQryStr -> the canonical Class name to address the aClass object
    returns:
        0 -> OK
        -1 -> ERR: Canonical Class name is already used a previous call of the method
            or MapXxxByName methods
    Fatal:
        if aClass or aQryStr is set to NULL or aQryStr is malformed
    */
    int (*set_class)(void *aThis, struct cfb_obj_lifecycle_iface_s *aClass, char *aQryStr);

    /*
    args:
        aClassName -> the canonical Class name to address the aClass object
    return:
        IClass Object that will use for creating instances of the class. This method
        will ALWAYS return a value.
    Fatal:
        if aQryStr is set to NULL or malformed
    */
    cfb_class_hnd_t (*get_class_hnd)(void *aThis, char *aClassName);

    /*
    Create a new instance or return a singleton instance of aClass depending on the
    implementation of the class
    args:
        aClass -> The IClass object returned by the get_class_hnd method
    return:
        a) address of the instance of the class if the aClass refers to a Concrete class
        b) NULL if the aClass is either an interface or undefined
    */
    void *(*get_obj_by_class_hnd)(void *aThis, cfb_class_hnd_t aClass);

    /*
    The method must be called after the last `set_class` invocation is made so that
    its `seals` the plugin from further `set_class` calls and invoke the `?` stmts
    call-backs.
     */
    void (*seal)(void *aThis);
};

#ifdef __cplusplus
extern "C"
{
#endif
    /*
    the function is provided by cfabric for the initialisation of the plugin to query other
    dependency.
    args:
        aQryStr: has the following syntax
            aQryStr = mod_name ['@' version [('-'|'<') version | '+']] )
            mod_name = ident
            version = num '.' num '.' num
            num = {0..9}+1
            such that:
                ('-'|'<') denotes INCLUSIVE and NON-INCLUSIVE range respectively
                '+' denotes the "at least" condition, that is, the LATEST plugin satisfying
                the condition
        aResult: return the plugin the satifies the query condition.
    return:
        0: Success on aQryStr
        -1: ERROR: malformed syntax.
        -2: ERROR: can't find plugins compatible version
        -3: ERROR: can't find name of plugin
    */
    typedef int (*cfb_plugin_getter_t)(char *aQryStr, cfb_plugin_t ***aResult);

    /**
     * The plugin developer provide the function of this interface to initilise itself
     * args:
     *  aPluginGetter: the function for accessing other plugins.
     *  aState: defines the purpose of the current execution such that 
     *      'i' means the module is being INSTALLED onto cfabric
     *      'l' means the module is being LOADED onto cfabric
     *      NOTE: the INSTALLED state: aPluginGetter implementation will download the required
     *      Module onto the cfabric platform but WON'T Load it! Hence the returnd plugin object
     *      is just an zombie version and not the real one provided by the dependency plugin.
     * return:
     *  0 Success
     *  -1: Error
    */
    typedef int (*cfb_init_plugin_cb_t)(cfb_plugin_getter_t aPluginGetter, char aState);

    /*this function allocate a new plugin object based on the rules define in aStmtStr
    ARGS:
        aQryStr: syntax: mod_name ['@' version]
        aResult: to the cfb_plugin_t ** variable to store the result of the newly created
            plugin object
        aCb: the callback function invoked when cfb_init_all_plugins() is called. Basically,
            this function does a list of calls to cfb_plugin_getter_t to get reference to all
            the plugins the current one it depends on, and then initialise all the class handle
            the it needs, perform other initialisation etc.
    return:
        0: Success
        -1: ERROR: malformed syntax on aStmtStr.
        -2: ERROR: conflicting verion. Check if there are another plugin with the same name and version number 
    */
    int cfb_define_plugin(char *aStmtStr, cfb_plugin_t ***aResult, cfb_init_plugin_cb_t aCb);
    /*...Also the definition below is to allow cfb_define_plugin to be passed to the plugin installer*/
    typedef int (*cfb_define_plugin_t)(char *aStmtStr, cfb_plugin_t ***aResult, cfb_init_plugin_cb_t aCb);

    /*
    when all the plugins are defined through cfb_define_plugin, call this function to initialise them
    return: 
        0: Success
        -1: ERROR: some plugins can't be initialise due to cfb_init_plugin_cb_t return -1
    */
    int cfb_init_all_plugins();
#ifdef __cplusplus
}
#endif
