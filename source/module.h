#pragma once

#include <stddef.h>

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
typedef int version_t;
typedef int cfb_class_hnd_t;
typedef struct cfb_plugin_s cfb_plugin_t;

/**
 * This interface is to be implemented by all classes
 * that can create objects (of itself) and/or delete those
 * objects
 **/
struct cfb_object_s
{
    /*called when object destruction*/
    void (*on_del)(void * /*constructure data structures*/);

    /*called when object instanciation*/
    void *(*on_new)(cfb_plugin_t **aCtx);
};

/**
 * The class as by plugin developers publish the plugin component and its
 * services to the cfabric matix.
 * <p>
 * This object is create internally by cfabric and is passed to the plugin
 * loading function.
 * 
 * @author David Chen Guofu
 */
struct cfb_plugin_s
{
    /**
     *This method maps the cfb_object_s for the class, which is
     *defined by the string name of the class
     *@param aClass the interface to the Implementation of the cfb_object_s interface. 
     *@param aClassName The canonical Class name to address the aClass object
     *@return 
     *  |Value|Description|
     *  |:--:|:--|
     *  |0|On success|
     *  |-1|ERR: duplicate call to this method.|
     */
    int (*decl_class)(void *aThis, struct cfb_object_s *aClass, char *aClassName);
    int (*decl_interface)(void *aThis, char *aClassName);

    /**
     *return the class handle based on its type string name 
     *@param aClassName The canonical Class name to address the aClass object
     *@return cfb_class_hnd_t for creating instances of the class. 
     *   > *NOTE:* This method returns nothing if no matching aClassName can be found
     *Fatal:
     *    if aQryStr is set to NULL or malformed
     **/
    cfb_class_hnd_t (*get_class_hnd)(void *aThis, char *aClassName);

    /**
     *Create a new instance or return a singleton instance of aClass depending on the
     *implementation of the class
     *@param aClassHnd -> The IClass object returned by the get_class_hnd method
     *@return
     *1. The instance of the class defined by aClassHnd
     *2. NULL if the aClassHnd is either an interface or undefined.
     */
    void *(*get_obj_by_class_hnd)(void *aThis, cfb_class_hnd_t aClassHnd);

    /**
     *The method must be called after the last `set_class` invocation is made so that
     *its *seals* the plugin from further `set_class` calls and invoke the `?` stmts
     *call-backs.
     */
    void (*seal)(void *aThis);
};

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     *the function is provided by cfabric for the initialisation of the plugin to query other
     *dependency.
     *@param aQryStr has the following syntax
      ```
            aQryStr = mod_name ['@' version [('-'|'<') version | '+']] )
            mod_name = ident
            version = num '.' num '.' num
            num = {0..9}+1
            such that:
                ('-'|'<') denotes INCLUSIVE and NON-INCLUSIVE range respectively
                '+' denotes the "at least" condition, that is, the LATEST plugin satisfying
                the condition
        ```
    @param aResult return the plugin the satifies the query condition.
    @return
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
    typedef int (*cfb_bind_plugin_cb_t)(cfb_plugin_getter_t aPluginGetter, char aState);

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
    int cfb_define_plugin(char *aStmtStr, cfb_plugin_t ***aResult, cfb_bind_plugin_cb_t aCb);
    /*...Also the definition below is to allow cfb_define_plugin to be passed to the plugin installer*/
    typedef int (*cfb_define_plugin_t)(char *aStmtStr, cfb_plugin_t ***aResult, cfb_bind_plugin_cb_t aCb);

    /*
    when all the plugins are defined through cfb_define_plugin, call this function to initialise them
    return: 
        0: Success
        -1: ERROR: some plugins can't be initialise due to cfb_bind_plugin_cb_t return -1
    */
    int cfb_init_all_plugins();
#ifdef __cplusplus
}
#endif

struct cfb_plugin_db_s
{
    /**
     * allocates a cfb_plugin_s object based on the parameters
     * @param aStmtStr name of the plugin including its version with the syntax `name@v1.v2.v3`
     * |part name| description|
     * |:--|:--|
     * |name | ident of the module
     * |v1 | major version : changes when deplicated functions are removed
     * |v2 | minor version : changes when new functions are added
     * |v3 | maintenance version: changes when implemenation of existing function changes
     * @param aPluging the cfb_plugin_s object to be published through cfabric
     * @param aCb The function to be called when `int cfb_init()` is called
     * @return
     * |value| description|
     * |:--|:--|
     * |0| on success
     * |-1 | dup plugin name+aVersion
     * |-2 | aResult is null
     **/
    int (*publish_plugin)(void **aThis, char *aStmtStr, cfb_plugin_t **aResult, cfb_bind_plugin_cb_t aCb);

    /**
     * returns a new cfb_plugin_t object
     **/
    cfb_plugin_t **(*new_plugin)(void **aThis);

    /**
     * This function initiate the binding of all plugins
     * @return 
     * |value| description|
     * |:--|:--|
     * |0| on success
     * |-1| some binding symbols were not found
     **/
    int (*bind_all_plugins)(void **aThis);
};

/*
The purpose of the class-builder module is to help CFabric developers
quickly implement objects via a common datastructure regime wrapped in
IDE friendly MACROS. 

However for the end-product Class objects to work properly, the
NS("cfabric").getInstanceByClassHandle("cfb_class_helper_s") should
be used to perform the following functions:
- CastToParent(IInterface **athis): for returning the base class address

*/

/**************************************************************
 *              GLOBAL Constants + utility macros                  
 **************************************************************/

/*
This constant defines the MAX number of interfaces a class can
implement. Currently a class can't implement more than 64
interfaces at the same time.
[WARNING]: If the number is too large, it may cause a stack over
flow. To understand why please refer the to CLASS IMPL section
macros
*/
#define IMPL_INTERFACE_MAX 64

/*Use this with generate identifier for ClassBuilder Create method
to create instance. The ident refers to the static variable that
points to the class definition*/
#define ClassInfo(name) \
    CLASS_STRUCT_FIELD_N(name)

#define ImplementClass(name, ctx) \
    CLASS_IMPL_FN_N(name)         \
    (ctx)

#define CLASS_INIT_FUNC(name) \
    struct cfb_object_s *CLASS_IMPL_FN_N(name)(cfb_plugin_t * *vCtx)

/*Type use for storing memory offset values*/
typedef size_t TMemOffset;
typedef void (*TClassInitFunc)(int /*aTableCounter*/);
//#define IType(name) struct INTERFACE_VTABLE_N(name)
/**************************************************************
 *                   Class Identifier Macros                  
 **************************************************************/

/**
 *Name of interface declaration
**/
#define INTERFACE_VTABLE_N(name) name
#define INTERFACE_FIELD_N(name) _##name##Vtable
#define INTERFACE_HDR_FIELD_N(name) _##name##VtableHdr

/**
 *name of the class definition type, this is the type
 *containing information on how an instance of the class
 *will be created
**/
#define CLASS_STRUCT_N(name) name##Class
#define CLASS_STRUCT_FIELD_N(name) name##ClassImpl
#define CLASS_STRUCT_IFIELD_N(name) name##Fields
#define CLASS_IMPL_FN_N(name) Impl##name##Class

/**************************************************************
 *                   Class Identifier Macros                  
 **************************************************************/
/*The data structure for each entry of the TypeCastMap*/
struct _classbuilder_TTypeCastMapEntry
{
    /*contains Cfabric handle id number of the class name
    */
    cfb_class_hnd_t handle;
    TMemOffset fieldOffset;
    void *ptrVtable;
};

/*The cast map is used in the class type object for tracking
the vTable-reference fieldOffset corresponding to the IInterface
vTable*/
struct _classbuilder_TTypeCastMap
{
    /*number of entries the map contains*/
    int entryCount;
    struct _classbuilder_TTypeCastMapEntry entries[0];
};

struct _classbuilder_TVTableRef
{
    /*points to the Vtable structure*/
    void *ptrVtable;
};

/*
It is defined BEFORE every vtable field of the class-instance 
*/
struct _classbuilder_TVTableHeader
{
    /*defines the offset of the current vTable relative to the 
    Header section of the Class object. 
    
    NOTE: the value is AWAYS negative since the header section
    comes BEFORE the vtable definitions*/
    TMemOffset hdrOffset;

    /*offset of the instance's vtable pointer relative to its
    data section*/
    TMemOffset instanceDataOffset;

    /*the index value of the castMap entry the vtable entry relates
    to. This value is use in calculating the base value of the object*/
    TMemOffset castMapEntryIdx;
};

/*Example of a concrete class implementation*/
struct _classbuilder_TClassHeader
{
    /*contains the address to the cast map*/
    struct _classbuilder_TTypeCastMap *ptrCastMap;

    /*To number of bytes needed to instanciate an instance
    of the object*/
    int instanceByteSize;
};

/**************************************************************
 *       [Internal] VTable-MAP Class Declaration Macros
 * 
 * Example:
 * 
 * BEGIN_CLASS_INTERFACES(name)
 * ... [Declare interface]
 *      IMPLEMENT(IMouseListner);
 *      IMPLEMENT(IDrawable);
 * ...
 * BEGIN_CLASS_INTERFACES
 *         
 **************************************************************/

#define IMPLEMENT(name)                                             \
    struct _classbuilder_TVTableHeader INTERFACE_HDR_FIELD_N(name); \
    struct INTERFACE_VTABLE_N(name) INTERFACE_FIELD_N(name)

#define BEGIN_CLASS_INTERFACES(name)                                 \
    static struct CLASS_STRUCT_N(name) * CLASS_STRUCT_FIELD_N(name); \
    struct CLASS_STRUCT_N(name)                                      \
    {                                                                \
        struct _classbuilder_TClassHeader header;                    \
        IMPLEMENT(cfb_object_s);                                     \
        IMPLEMENT(name);

#define END_CLASS                              \
    struct _classbuilder_TTypeCastMap castMap; \
    }                                          \
    ;

/**************************************************************
 *     [Internal] CLASS IMPLEMENTATION Declaration Macros
 * 
 * Example:
 * 
 * BEGIN_CLASS_IMPLEMENTATION(className)
 * ... [map method call]
 *      MAP_METHOD(Say);
 * ...
 * MAP_INTERFACE(IHello)
 * ...
 *      MAP_CALL(say, _imp_hellosay);
 * ...
 * MAP_INTERFACE(IGoodBye)
 * ...
 *      MAP_CALL(say, _imp_goodbyesay);
 * ...
 * END_CLASS_IMPLEMENTATION
 *         
 **************************************************************/
#define _BEGIN_METHOD_MAP(name)              \
    if (vTableCounter >= IMPL_INTERFACE_MAX) \
        return;                              \
    _InitEntries[vTableCounter] = ({                                                       \
        void __fn__(int aTableCounter)                                                     \
        {                                                                                  \
            __auto_type _classMethodPtr = &(*_classImplPtr)->INTERFACE_FIELD_N(name);      \
            void *_classMethodHdrPtr = &(*_classImplPtr)->INTERFACE_HDR_FIELD_N(name);\
            (*_classImplPtr)->INTERFACE_HDR_FIELD_N(name).hdrOffset = -(TMemOffset)((size_t)(*_classImplPtr)-(size_t)_classMethodHdrPtr);\
            (*_classImplPtr)->INTERFACE_HDR_FIELD_N(name).instanceDataOffset = vInstanceDataOffset - (aTableCounter * sizeof(struct _classbuilder_TVTableRef));\
            (*_classImplPtr)->INTERFACE_HDR_FIELD_N(name).castMapEntryIdx = aTableCounter;\
            (*_classImplPtr)->castMap.entries[aTableCounter] = (struct _classbuilder_TTypeCastMapEntry){\
                .handle = vCtx == NULL ? -(vTableCounter+1) : (*vCtx)->get_class_hnd(vCtx, #name),\
                .fieldOffset = (aTableCounter-1) * sizeof(struct _classbuilder_TVTableRef),\
                .ptrVtable = _classMethodPtr,\
            };

#define MAP_CALL_TO_METHOD(src, dst) \
    _classMethodPtr->src = dst

#define MAP_METHOD(method) \
    MAP_CALL_TO_METHOD(method, method)

#define _END_METHOD_MAP \
    }                   \
    __fn__;             \
    });                 \
    vTableCounter++

#define MAP_INTERFACE(name) \
    _END_METHOD_MAP;        \
    _BEGIN_METHOD_MAP(name)

#define BEGIN_CLASS_IMPLEMENTATION(name)                                                         \
                                                                                                 \
    CLASS_INIT_FUNC(name)                                                                        \
    {                                                                                            \
        size_t vTableCounter = 0;                                                                \
        int instanceDataSize = 0;                                                                \
        __auto_type _className = ClassName(name);                                                \
        __auto_type _classImplSize = sizeof(struct CLASS_STRUCT_N(name));                        \
        __auto_type _instanceDataSectionSize = sizeof(struct CLASS_STRUCT_IFIELD_N(name));       \
        __auto_type _classImplPtr = &CLASS_STRUCT_FIELD_N(name);                                 \
        __auto_type _vtable_base = (size_t) & (*_classImplPtr)->INTERFACE_FIELD_N(cfb_object_s); \
        if (*_classImplPtr)                                                                      \
            return &(*_classImplPtr)->INTERFACE_FIELD_N(cfb_object_s);                           \
        int vInstanceDataOffset;                                                                 \
        TClassInitFunc _InitEntries[IMPL_INTERFACE_MAX];                                         \
        _BEGIN_METHOD_MAP(cfb_object_s)                                                          \
        {                                                                                        \
            MAP_METHOD(on_del);                                                                  \
            MAP_METHOD(on_new);                                                                  \
        }                                                                                        \
        MAP_INTERFACE(name)

#define END_CLASS_IMPLEMENTATION                                                                                                          \
    _END_METHOD_MAP;                                                                                                                      \
    (*_classImplPtr) = calloc(1, _classImplSize + (vTableCounter * sizeof(struct _classbuilder_TTypeCastMapEntry)));                      \
    (*_classImplPtr)->header.ptrCastMap = &(*_classImplPtr)->castMap;                                                                     \
    vInstanceDataOffset = (vTableCounter) * sizeof(struct _classbuilder_TVTableRef);                                                      \
    (*_classImplPtr)->header.instanceByteSize = _instanceDataSectionSize + vInstanceDataOffset - sizeof(struct _classbuilder_TVTableRef); \
    (*_classImplPtr)->castMap.entryCount = vTableCounter;                                                                                 \
    for (int i = 0; i < vTableCounter; i++)                                                                                               \
    {                                                                                                                                     \
        _InitEntries[i](i);                                                                                                               \
    }                                                                                                                                     \
    __auto_type ptrConstructor = &(*_classImplPtr)->INTERFACE_FIELD_N(cfb_object_s);                                                      \
    if (vCtx != NULL)                                                                                                                     \
        (*vCtx)                                                                                                                           \
            ->decl_class(vCtx, ptrConstructor, _className);                                                                               \
    return ptrConstructor;                                                                                                                \
    }

struct cfb_class_helper_s
{
    void *(*get_obj_data)(void **aObj);
    void *(*alloc_obj)(struct _classbuilder_TClassHeader *aClassHdr);
    void *(*del_obj)(void ***aObj);
    void *(*get_interface)(void **aObj, cfb_class_hnd_t vClassHandle);
};

struct cfb_class_helper_s **cfb_class_helper();
struct cfb_plugin_db_s **cfb_plugin_db();