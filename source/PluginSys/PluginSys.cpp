#include <stdio.h>
#include <windows.h>
#include "PluginSys.h"

typedef struct
{
    /*
    0 -> undefined
    1 -> concrete class
    2 -> interface class
     */
    int Type;

    /*
    the vector index value where to reference to
    the class methods are stored
    */
    int Offset;
} IClassEntry;

#ifdef __cplusplus
#define EXPORT __declspec(dllexport)

extern "C"
{
#endif

    EXPORT INamespaceContext **OpenNamespace(char *aName);
#ifdef __cplusplus
}
#endif

#include <map>
#include <vector>
#include <string>

typedef std::map<std::string, IClassEntry> TClassEntryMap;
typedef std::map<std::string, void *> TNamespaceEntryMap;
typedef std::vector<void *> TClassEntryVector;

typedef struct
{
    INamespaceContext *Methods;

    /*
    Indexes the canonical classnames of TClassEntryVector Classes. Note that, the double
    indirection is done so that to elimimate the need for the classes to be load 
    some prescribed sequence.
     */
    TClassEntryMap ClassNameIdx;
    TClassEntryVector *Classes;
    //int InterfaceIdCount;
} INamespaceContextImpl;

TClassEntryVector gClasses;

int _imp_MapClassOrInterfaceByName(void *aThis, IObject *aClass, char *aName)
{

    IClassEntry Entry;
    INamespaceContextImpl *vThis = (INamespaceContextImpl *)aThis;
    if (vThis->ClassNameIdx.find(aName) != vThis->ClassNameIdx.end())
    {
        Entry = vThis->ClassNameIdx[aName];
        if (Entry.Type != 0)
        {
            return -1;
        }
        Entry.Type = aClass == NULL ? 2 : 1;
        (*(vThis->Classes))[Entry.Offset] = aClass;
    }
    else
    {
        Entry.Type = 1;
        Entry.Offset = vThis->Classes->size();
        vThis->Classes->push_back(aClass);
    }
    vThis->ClassNameIdx[aName] = Entry;
    return 0;
}

TClassHandle _imp_GetClassByName(void *aThis, char *aName)
{
    INamespaceContextImpl *vThis = (INamespaceContextImpl *)aThis;
    if (vThis->ClassNameIdx.find(aName) != vThis->ClassNameIdx.end())
    {
        return vThis->ClassNameIdx[aName].Offset;
    }
    int vOffset = vThis->Classes->size();
    vThis->ClassNameIdx[aName] = (IClassEntry){.Type = 0, .Offset = vOffset};
    vThis->Classes->push_back(NULL);
    return vOffset;
}

void *_imp_InstanceByClassHandle(void *aThis, TClassHandle aClassHandle)
{
    INamespaceContextImpl *vThis = (INamespaceContextImpl *)aThis;
    if (0 <= aClassHandle && aClassHandle < vThis->Classes->size())
    {
        IObject *vClassDef = (IObject *)(*(vThis->Classes))[aClassHandle];
        if (vClassDef != NULL)
        {
            return vClassDef->Constructor((INamespaceContext **)vThis);
        }
    }
    return NULL;
}

INamespaceContext _impl = {
    .MapClassByName = _imp_MapClassOrInterfaceByName,
    .GetClassHandleByName = _imp_GetClassByName,
    .GetInstanceByClassHandle = _imp_InstanceByClassHandle};

TNamespaceEntryMap _nsMap;
/*
*/
EXPORT INamespaceContext **
OpenNamespace(char *aName)
{
    INamespaceContext **vResult;

    if (_nsMap.find(aName) != _nsMap.end())
    {
        vResult = (INamespaceContext **)_nsMap[aName];
    }
    else
    {
        INamespaceContextImpl *vNsObj = new INamespaceContextImpl();
        vNsObj->Methods = (INamespaceContext *)&_impl;
        vNsObj->Classes = &gClasses;
        _nsMap[aName] = vNsObj;
        vResult = (INamespaceContext **)&vNsObj->Methods;
    }

    return vResult;
}