/*
The purpose of the class-builder module is to help CFabric developers
quickly implement objects via a common datastructure regime wrapped in
IDE friendly MACROS. 

However for the end-product Class objects to work properly, the
NS("cfabric").getInstanceByClassHandle("cfb_class_helper_s") should
be used to perform the following functions:
- CastToParent(IInterface **athis): for returning the base class address

*/
#pragma once

#include <stddef.h>
#include "cfabric.h"

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

/*Type use for storing memory offset values*/
typedef int TMemOffset;
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
    int instanceDataOffset;

    /*the index value of the castMap entry the vtable entry relates
    to. This value is use in calculating the base value of the object*/
    int castMapEntryIdx;
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
 * NOTE:
 *      - "name" must be same for both BEGIN_CLASS_INTERFACES and 
 *          IMPLEMENTS
 **************************************************************/

#define IMPLEMENT(name)                                             \
    struct _classbuilder_TVTableHeader INTERFACE_HDR_FIELD_N(name); \
    struct INTERFACE_VTABLE_N(name) INTERFACE_FIELD_N(name)

#define BEGIN_CLASS_INTERFACES(name)                                 \
    static struct CLASS_STRUCT_N(name) * CLASS_STRUCT_FIELD_N(name); \
    struct CLASS_STRUCT_N(name)                                      \
    {                                                                \
        struct _classbuilder_TClassHeader header;                    \
        IMPLEMENT(cfb_obj_lifecycle_iface_s);                        \
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
            (*_classImplPtr)->INTERFACE_HDR_FIELD_N(name).hdrOffset = -(TMemOffset)(_classMethodHdrPtr - (size_t)(*_classImplPtr));\
            (*_classImplPtr)->INTERFACE_HDR_FIELD_N(name).instanceDataOffset = vInstanceDataOffset - (aTableCounter * sizeof(struct _classbuilder_TVTableRef));\
            (*_classImplPtr)->INTERFACE_HDR_FIELD_N(name).castMapEntryIdx = aTableCounter;\
            (*_classImplPtr)->castMap.entries[aTableCounter] = (struct _classbuilder_TTypeCastMapEntry){\
                .handle = (*vCtx)->get_class_hnd(vCtx, #name),\
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

#define BEGIN_CLASS_IMPLEMENTATION(name)                                                                      \
    static struct CLASS_STRUCT_N(name) * CLASS_STRUCT_FIELD_N(name);                                          \
    void *CLASS_IMPL_FN_N(name)(cfb_plugin_t * *vCtx)                                                         \
    {                                                                                                         \
        size_t vTableCounter = 0;                                                                             \
        int instanceDataSize = 0;                                                                             \
        __auto_type _className = ClassName(name);                                                             \
        __auto_type _classImplSize = sizeof(struct CLASS_STRUCT_N(name));                                     \
        __auto_type _instanceDataSectionSize = sizeof(struct CLASS_STRUCT_IFIELD_N(name));                    \
        __auto_type _classImplPtr = &CLASS_STRUCT_FIELD_N(name);                                              \
        __auto_type _vtable_base = (size_t) & (*_classImplPtr)->INTERFACE_FIELD_N(cfb_obj_lifecycle_iface_s); \
        int vInstanceDataOffset;                                                                              \
        TClassInitFunc _InitEntries[IMPL_INTERFACE_MAX];                                                      \
        _BEGIN_METHOD_MAP(cfb_obj_lifecycle_iface_s)                                                          \
        {                                                                                                     \
            MAP_METHOD(on_del);                                                                               \
            MAP_METHOD(on_new);                                                                               \
        }                                                                                                     \
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
    void *ptrConstructor = (void *)(*_classImplPtr) + _vtable_base;                                                                       \
    (*vCtx)                                                                                                                               \
        ->set_class(vCtx, ptrConstructor, _className);                                                                                    \
    return ptrConstructor;                                                                                                                \
    }

/**************************************************************
 *       Interface of Class builder that is register under
 *       then Cfabric Namespace
 *         
 **************************************************************/
struct cfb_class_helper_s
{
    void *(*get_obj_data)(void **aObj);
    void *(*alloc_obj)(struct _classbuilder_TClassHeader *aClassHdr);
    void *(*del_obj)(void ***aObj);
    void *(*get_interface)(void **aObj, cfb_class_hnd_t vClassHandle);
};
