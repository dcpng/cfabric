#include <string.h>
#include <windows.h>
#include <stdio.h>
#include "../../PluginSys.h"
#include "../../../TClassBuilder/TClassBuilder.h"

/*
********** Section defining the test class *************
*/

struct IHelloVTable
{
    IInterface baseClass; /*must be the first field of the object*/

    /**/
    void (*say)(void * /*this*/, char * /*output buffer*/);
};

struct IGoodByeVTable
{
    IInterface baseClass; /*must be the first field of the object*/

    /**/
    void (*say)(void * /*this*/, char * /*output buffer*/);
};

struct TAppleVTable
{
    IObject base; /*must be the first field of the object*/

    /**/
    void (*say)(void * /*this*/, char * /*output buffer*/);
};

/*
struct TAppleStaticFields
{
    TOpenNamespaceFn _ns; // the name space function to get the ClassBuild Instance
    int isCreated;
};*/

struct TAppleClass
{
    /**
     * Must be the first field of the TAppleClass instance
     **/
    struct TClassHeader header;
    /**
     * return the static fields of the class
     **/
    //struct TAppleStaticFields fields;

    /*This place holder is use for defining
    the offset of instance fields*/
    /**
     * The base object
     **/
    struct TAppleVTable _TAppleVTable;

    struct TAppleInstanceFields
    {
        char *helloData, *goodByeData;
    } * fieldPlaceHolder;

    /**
     * Examples of interface implemetations
     **/
    struct IHelloVTable _IHelloVTable;
    struct IGoodByeVTable _IGoodByeVTable;

    /*ALWAYS the LAST FIELD: The cast table
    of the base cast to function properly*/
    struct TTypeCastMap castMap;
};

#define CastToParent(this) \
    (*((void **)(((void *)this) + sizeof(void *))))

/*The static class type object*/
struct TAppleClass *TAppleClassImpl;

void *GetInstanceData(IInterface **aThis)
{
    void *vInstanceBase = (*aThis)->Cast(aThis, IObjectHandler);
    struct TClassHeader *vInfo = CastToParent(vInstanceBase);
    return vInstanceBase + vInfo->dataOffset;
}

struct TTypeCastMap *GetCastMap(IInterface **aThis)
{
    void *vInstanceBase = (*aThis)->Cast(aThis, IObjectHandler);
    struct TClassHeader *vInfo = CastToParent(vInstanceBase);
    return vInfo->ptrCastMap;
}

void *CreateInstance(struct TClassHeader *aClassHdr)
{
    struct TVTableRef *vTableRef = calloc(1, aClassHdr->instanceByteSize);
    vTableRef[0].ptrParent = aClassHdr;
    vTableRef[0].ptrVtable = aClassHdr->ptrCastMap->entries[0].ptrVtable;
    for (
        int i = 1, j = aClassHdr->ptrCastMap->entryCount;
        i < j;
        i++)
    {
        vTableRef[i].ptrParent = &vTableRef[0].ptrVtable;
        vTableRef[i].ptrVtable = aClassHdr->ptrCastMap->entries[i].ptrVtable;
    }
    return vTableRef;
}

/*This works because the instance's memory layout it structured such that
offset value is stored on the instance object just before the *This pointer*/
void *CastInterface(IInterface **aThis, TClassHandle vClassHandle)
{
    aThis = (IInterface **)CastToParent(aThis);
    return (*aThis)->Cast(aThis, vClassHandle);
}

void *DoCast(void *aThis, TClassHandle vClassHandle)
{
    /*the -1 value is the label for casting to the instance's base-class*/
    if (vClassHandle == IObjectHandler)
    {
        return aThis;
    }
    __auto_type vCastmap = GetCastMap(aThis);
    for (int i = 0; i < vCastmap->entryCount; i++)
    {
        struct TTypeCastMapEntry *vEntry = &vCastmap->entries[i];
        if (vEntry->handle == vClassHandle)
        {
            return (void *)aThis + vEntry->fieldOffset;
        }
    }
}

void _impl_HelloSay(IInterface **aThis, char *aOutput)
{
    __auto_type vFields = (struct TAppleInstanceFields *)GetInstanceData(aThis);
    sprintf(aOutput, "data = %s", vFields->helloData);
}

void _impl_GoodByeSay(IInterface **aThis, char *aOutput)
{
    __auto_type vFields = (struct TAppleInstanceFields *)GetInstanceData(aThis);
    sprintf(aOutput, "data = %s", vFields->goodByeData);
}

void *_impl_constructor(INamespaceContext **aCtx)
{
    void *vObj = CreateInstance(TAppleClassImpl);
    __auto_type vFields = (struct TAppleInstanceFields *)GetInstanceData(vObj);
    vFields->goodByeData = "Good Bye";
    vFields->helloData = "Hello";
    return vObj;
}

void _impl_destructor(IInterface **aThis)
{
    void *vThis = (*aThis)->Cast(aThis, IObjectHandler);
    free(vThis);
    *aThis = 0;
}

typedef void (*TClassInitFunc)(int /*aTableCounter*/);
void ImplementTApple(INamespaceContext **vCtx, TOpenNamespaceFn aNsFn)
{
    size_t vTableCounter = 0;
    int instanceDataSize = 0;
    __auto_type _className = "TApple";
    __auto_type _classImplSize = sizeof(struct TAppleClass);
    __auto_type _instanceDataSectionSize = sizeof(struct TAppleInstanceFields);
    __auto_type _classImplPtr = &TAppleClassImpl;
    __auto_type _vtable_base = (long)&(*_classImplPtr)->_TAppleVTable;
#define CAST_BUFFER_SIZE 64
    TClassInitFunc _InitEntries[CAST_BUFFER_SIZE];

    if (vTableCounter >= CAST_BUFFER_SIZE)
        return;
    _InitEntries[vTableCounter] = ({
        void __fn__(int aTableCounter)
        {
            /*Part of the Base header*/
            __auto_type _classMethodPtr = &(*_classImplPtr)->_TAppleVTable;
            (*_classImplPtr)->castMap.entries[aTableCounter] = (struct TTypeCastMapEntry){
                .handle = (*vCtx)->GetClassHandleByName(vCtx, "TApple"),
                .fieldOffset = aTableCounter * sizeof(struct TVTableRef),
                .ptrVtable = _classMethodPtr,
            };
            _classMethodPtr->base.baseClass.Cast = DoCast;
            _classMethodPtr->base.Destructor = _impl_destructor;
            _classMethodPtr->base.Constructor = _impl_constructor;
        }
        __fn__;
    });
    vTableCounter++;

    if (vTableCounter >= CAST_BUFFER_SIZE)
        return;
    _InitEntries[vTableCounter] = ({
        void __fn__(int aTableCounter)
        {
            __auto_type _classMethodPtr = &(*_classImplPtr)->_IHelloVTable;
            (*_classImplPtr)->castMap.entries[aTableCounter] = (struct TTypeCastMapEntry){
                .handle = (*vCtx)->GetClassHandleByName(vCtx, "IHello"),
                .fieldOffset = aTableCounter * sizeof(struct TVTableRef),
                .ptrVtable = _classMethodPtr,
            };
            _classMethodPtr->baseClass.Cast = CastInterface;
            _classMethodPtr->say = _impl_HelloSay;
        }
        __fn__;
    });
    vTableCounter++;

    if (vTableCounter >= CAST_BUFFER_SIZE)
        return;
    _InitEntries[vTableCounter] = ({
        void __fn__(int aTableCounter)
        {
            __auto_type _classMethodPtr = &(*_classImplPtr)->_IGoodByeVTable;
            (*_classImplPtr)->castMap.entries[aTableCounter] = (struct TTypeCastMapEntry){
                .handle = (*vCtx)->GetClassHandleByName(vCtx, "IGoodBye"),
                .fieldOffset = aTableCounter * sizeof(struct TVTableRef),
                .ptrVtable = _classMethodPtr,
            };
            _classMethodPtr->baseClass.Cast = CastInterface;
            _classMethodPtr->say = _impl_GoodByeSay;
        }
        __fn__;
    });
    vTableCounter++;

    (*_classImplPtr) = calloc(1, _classImplSize + (vTableCounter * sizeof(struct TTypeCastMapEntry)));
    (*_classImplPtr)->header.ptrCastMap = &(*_classImplPtr)->castMap;
    (*_classImplPtr)->header.dataOffset = (vTableCounter * sizeof(struct TVTableRef));
    (*_classImplPtr)->header.instanceByteSize = _instanceDataSectionSize + (*_classImplPtr)->header.dataOffset;
    (*_classImplPtr)->header.ns = aNsFn;
    (*_classImplPtr)->castMap.entryCount = vTableCounter;
    for (int i = 0; i < vTableCounter; i++)
    {
        _InitEntries[i](i);
    }
    (*vCtx)->MapClassByName(vCtx, (char *)(*_classImplPtr) + _vtable_base, _className);
}

void Install(INamespaceContext **vCtx, TOpenNamespaceFn aNsFn)
{
    ImplementTApple(vCtx, aNsFn);
}
/*
********************************************************
*/

HANDLE vDllHandle, vClassBuilderDllHandle, vCollectionDllHandle;
TModInstaller vInstaller;
TOpenNamespaceFn vNamespace;
INamespaceContext **vCtx, **vCtx2;
TClassHandle vClsHandle1, vClsHandle2, hApple, hCollection;
struct TCollection **vInstance2;
IObject **vInstance;
struct IGoodByeVTable **vGoodBye;
struct IHelloVTable **vHello;

#include <unit-test/bdd-for-c.h>

__declspec(dllexport) void Run()
{
    main();
}

spec("PluginSys.dll")
{

    it("should load the Plugin Context")
    {
        vDllHandle = LoadLibrary(TEXT("../PluginSys/.makefiles/debug/PluginSys.debug.dll"));
        check(vDllHandle != 0);
    }

    it("should load the CreateContext and install the TClassBuilder.dll plugin")
    {
        vNamespace = (TOpenNamespaceFn)GetProcAddress(vDllHandle, "OpenNamespace");
        check(vNamespace != 0);
    }

    it("should assign a non-NULL pointer value from calling vNamespace")
    {
        vCtx = vNamespace("pg.ac.unitech");
        check(vCtx != 0);
    }

    it("should assign same non-NULL pointer on same NS")
    {
        check(vCtx == vNamespace("pg.ac.unitech"));
    }

    it("should assign different non-NULL pointer on different NS")
    {
        check(vCtx != vNamespace("pg.ac.unitech2"));
    }

    it("handlers are SAME from GetClassHandleByName when the classnames are SAME")
    {
        vClsHandle1 = (*vCtx)->GetClassHandleByName(vCtx, "ICLASS");
        vClsHandle2 = (*vCtx)->GetClassHandleByName(vCtx, "ICLASS");
        check(vClsHandle1 == vClsHandle1);
    }

    it("handlers are DIFFERENT from GetClassHandleByName when the classnames are DIFFERENT")
    {
        vClsHandle1 = (*vCtx)->GetClassHandleByName(vCtx, "ICLASS1");
        vClsHandle2 = (*vCtx)->GetClassHandleByName(vCtx, "ICLASS2");
        check(vClsHandle1 == vClsHandle1);
    }

    it("Installing Plugin class")
    {
        Install(vCtx, vNamespace);
        hApple = (*vCtx)->GetClassHandleByName(vCtx, "TApple");
    }

    it("Test: Class instance creation")
    {
        vInstance = (*vCtx)->GetInstanceByClassHandle(vCtx, hApple);
        check(vInstance != NULL);
    }

    it("Test: Class casting")
    {
        TClassHandle hHello = (*vCtx)->GetClassHandleByName(vCtx, "IHello");
        TClassHandle hGoodBye = (*vCtx)->GetClassHandleByName(vCtx, "IGoodBye");

        /*test casting from base -> Interface*/
        vHello = (*vInstance)->baseClass.Cast(vInstance, hHello);
        check(vHello != NULL);
        vGoodBye = (*vInstance)->baseClass.Cast(vInstance, hGoodBye);
        check(vGoodBye != NULL);

        /*test casting from InterfaceA <-> InterfaceB*/
        void *vPtr = (*vHello)->baseClass.Cast(vHello, hGoodBye);
        check(vPtr == vGoodBye);

        vPtr = (*vGoodBye)->baseClass.Cast(vGoodBye, hHello);
        check(vPtr == vHello);

        /*test casting from InterfaceA -> InterfaceA*/
        vPtr = (*vGoodBye)->baseClass.Cast(vGoodBye, hGoodBye);
        check(vPtr == vGoodBye);

        vPtr = (*vHello)->baseClass.Cast(vHello, hHello);
        check(vPtr == vHello);

        /*test casting from Interface -> base*/
        vPtr = (*vHello)->baseClass.Cast(vHello, hApple);
        check(vPtr == vInstance);

        vPtr = (*vGoodBye)->baseClass.Cast(vGoodBye, hApple);
        check(vPtr == vInstance);
    }

    it("Test: method call")
    {
        char vResult[64];
        (*vHello)->say(vHello, vResult);
        check(strncmp(vResult, "data = Hello", 64) == 0);

        (*vGoodBye)->say(vGoodBye, vResult);
        check(strncmp(vResult, "data = Good Bye", 64) == 0);
    }
}