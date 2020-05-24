/*
The purpose of the class-builder module is to help CFabric developers
quickly implement objects via a common datastructure regime wrapped in
IDE friendly MACROS. 

However for the end-product Class objects to work properly, the
NS("cfabric").GetInstanceByClassHandle("TClassBuilder") should
be used to perform the following functions:
- CastToParent(IInterface **athis): for returning the base class address

*/

#ifndef TCLASSBUILDER_H
#define TCLASSBUILDER_H

#include <stddef.h>
#include "../PluginSys/PluginSys.h"

/**************************************************************
 *              GLOBAL Constants + utility macros                  
 **************************************************************/

/*
This constant defines the MAX number of interfaces a class can
implement. Currently a class can't implement more than 64
interfaces at the same time.
[WARNING]: If the number is too large, if may cause a stack over
flow. To understand why please refer the to CLASS IMPL section
macros
*/
#define IMPL_INTERFACE_MAX 64

/*Use this with generate identifier for ClassBuilder Create method
to create instance. The ident refers to the global variable that
points to the class definition*/
#define ClassInfo(name) \
    CLASS_STRUCT_FIELD_N(name)
#define ImplementClass(name, ctx, ns) \
    CLASS_IMPL_FN_N(name)             \
    (ctx, ns)

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

/**
 *name of the class definition type, this is the type
 *containing information on how an instance of the class
 *will be created
**/
#define CLASS_STRUCT_N(name) name##Class
#define CLASS_STRUCT_FIELD_N(name) name##ClassImpl
//#define CLASS_STRUCT_SFIELD_N(name) name##StaticFields
#define CLASS_STRUCT_IFIELD_N(name) name##Fields
#define CLASS_IMPL_FN_N(name) Impl##name##Class

/**
 *name of the instance data section. It is the section
 *of the instance storing the state of the object.
**/
//#define IDATA_STRUCT_N(name) name##Fields

/**
 *name of the instance data section. It is the section
 *of the instance storing the state of the object.
**/
//#define SDATA_STRUCT_N(name) name##SFields

/**************************************************************
 *                   Class Identifier Macros                  
 **************************************************************/
/*The data structure for each entry of the TypeCastMap*/
struct TTypeCastMapEntry
{
    /*contains Cfabric handle id number of the class name
    */
    TClassHandle handle;
    TMemOffset fieldOffset;
    void *ptrVtable;
};

/*The cast map is used in the class type object for tracking
the vTable-reference fieldOffset corresponding to the IInterface
vTable*/
struct TTypeCastMap
{
    /*number of entries the map contains*/
    int entryCount;
    struct TTypeCastMapEntry entries[0];
};

struct TVTableRef
{
    /*points to the Vtable structure*/
    void *ptrVtable;
    /*contains the address of base vtable refer from the
    base address of the object*/
    void *ptrParent;
};

/*Example of a concrete class implementation*/
struct TClassHeader
{
    /*contains the address offset of where the data section
    of the body is stored*/
    TMemOffset dataOffset;

    /*points to the name space function to access other plugins*/
    TOpenNamespaceFn ns;

    /*contains the address to the cast map*/
    struct TTypeCastMap *ptrCastMap;

    /*To number of bytes needed to instanciate an instance
    of the object*/
    int instanceByteSize;
};

/**************************************************************
 *                 Method-Declaration Macros                  
 **************************************************************/
//#define METHOD(name) \
//    (*name)

//#define NO_ARG \
//    (IInterface * *aThis)

//#define ARG(...) \
//    (IInterface * *aThis, __VA_ARGS__)

/**************************************************************
 *              Class-Vtable Declaration Macros               
 * 
 * Example:
 * 
 * struct className {
 * ... DECLARE_INTERFACE
 *      void (*m1) (IInterface ** athis);
 *      int  (*m2) (IInterface ** athis, int, ...);
 * ...
 * }
 * 
 **************************************************************/
#define DECLARE_INTERFACE \
    IInterface baseClass

#define DECLARE_CLASS \
    IObject base

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

#define BEGIN_CLASS_INTERFACES(name)                                 \
    extern struct CLASS_STRUCT_N(name) * CLASS_STRUCT_FIELD_N(name); \
    struct CLASS_STRUCT_N(name)                                      \
    {                                                                \
        struct TClassHeader header;                                  \
        struct CLASS_STRUCT_IFIELD_N(name) * fieldPlaceHolder;       \
        struct INTERFACE_VTABLE_N(name) INTERFACE_FIELD_N(name);

#define IMPLEMENT(name) \
    struct INTERFACE_VTABLE_N(name) INTERFACE_FIELD_N(name)

#define END_CLASS                \
    struct TTypeCastMap castMap; \
    }                            \
    ;

/**************************************************************
 *     [Internal] CLASS IMPLEMENTATION Declaration Macros
 * 
 * Example:
 * 
 * BEGIN_CLASS_IMPLEMENTATION(className)
 * ... [map object function calls]
 *      MAP_OBJ_CAST_CALL(_imp_cast);
 *      MAP_OBJ_DSTR_CALL(_imp_dstr);
 *      MAP_OBJ_CSTR_CALL(_imp_cstr);
 * ... [map method call]
 *      MAP_OBJ_CALL(say, _imp_say);
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
            (*_classImplPtr)->castMap.entries[aTableCounter] = (struct TTypeCastMapEntry){ \
                .handle = (*vCtx)->GetClassHandleByName(vCtx, #name),                            \
                .fieldOffset = aTableCounter * sizeof(struct TVTableRef),                  \
                .ptrVtable = _classMethodPtr,                                              \
            };

#define MAP_CALL(src, dst) \
    _classMethodPtr->src = dst

#define _END_METHOD_MAP \
    }                   \
    __fn__;             \
    });                 \
    vTableCounter++;

#define BEGIN_CLASS_IMPLEMENTATION(name)                                                   \
    struct CLASS_STRUCT_N(name) * CLASS_STRUCT_FIELD_N(name);                              \
    void CLASS_IMPL_FN_N(name)(INamespaceContext * *vCtx, TOpenNamespaceFn aNsFn)          \
    {                                                                                      \
        int vTableCounter = 0;                                                             \
        int instanceDataSize = 0;                                                          \
        __auto_type _className = #name;                                                    \
        __auto_type _classImplSize = sizeof(struct CLASS_STRUCT_N(name));                  \
        __auto_type _instanceDataSectionSize = sizeof(struct CLASS_STRUCT_IFIELD_N(name)); \
        __auto_type _classImplPtr = &CLASS_STRUCT_FIELD_N(name);                           \
        __auto_type _vtable_base = (long)&(*_classImplPtr)->INTERFACE_FIELD_N(name);       \
        TClassInitFunc _InitEntries[IMPL_INTERFACE_MAX];                                   \
        _BEGIN_METHOD_MAP(name)

#define MAP_OBJ_CAST_CALL(dst) \
    MAP_CALL(base.baseClass.Cast, dst)

#define MAP_CAST_CALL(dst) \
    MAP_CALL(baseClass.Cast, dst)

#define MAP_OBJ_DSTR_CALL(dst) \
    MAP_CALL(base.Destructor, dst)

#define MAP_OBJ_CSTR_CALL(dst) \
    MAP_CALL(base.Constructor, dst)

#define MAP_INTERFACE(name) \
    _END_METHOD_MAP _BEGIN_METHOD_MAP(name)

#define END_CLASS_IMPLEMENTATION                                                                                      \
    _END_METHOD_MAP(*_classImplPtr) = calloc(1, _classImplSize + (vTableCounter * sizeof(struct TTypeCastMapEntry))); \
    (*_classImplPtr)->header.ptrCastMap = &(*_classImplPtr)->castMap;                                                 \
    (*_classImplPtr)->header.dataOffset = (vTableCounter * sizeof(struct TVTableRef));                                \
    (*_classImplPtr)->header.instanceByteSize = _instanceDataSectionSize + (*_classImplPtr)->header.dataOffset;       \
    (*_classImplPtr)->header.ns = aNsFn;                                                                              \
    (*_classImplPtr)->castMap.entryCount = vTableCounter;                                                             \
    for (int i = 0; i < vTableCounter; i++)                                                                           \
    {                                                                                                                 \
        _InitEntries[i](i);                                                                                           \
    }                                                                                                                 \
    (*vCtx)->MapClassByName(vCtx, (char *)(*_classImplPtr) + _vtable_base, _className);                               \
    }

/**************************************************************
 *       Interface of Class builder that is register under
 *       then Cfabric Namespace
 *         
 **************************************************************/
typedef struct
{
    IObject base;
    void *(*getFields)(IInterface **aThis);
    void *(*createInstance)(struct TClassHeader *aClassHdr);
    void *(*getInterface)(IInterface **aThis, TClassHandle vClassHandle);
    void *(*rootCastOp)(void *aThis, TClassHandle vClassHandle)
} TClassBuilder;

#endif