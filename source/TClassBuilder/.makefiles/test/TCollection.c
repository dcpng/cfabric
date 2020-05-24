#include "TCollection.h"
#include "IStack.h"

struct TCollectionFields
{
    void *buffer[10];
    int cursor;
};

struct TCollectionStaticFields
{
    TClassBuilder *gClassBuilder;
    TClassHandle iStack;
    TClassHandle iClassBuilder;
};

struct TCollectionStaticFields gSF;

BEGIN_CLASS_INTERFACES(TCollection)
IMPLEMENT(IStack);
END_CLASS

void *_impl_cast(IInterface **aThis, TClassHandle aClassHandle)
{
    return gSF.gClassBuilder->rootCastOp(aThis, aClassHandle);
}

void _impl_dstr(IInterface **aThis)
{
}

void _impl_cstr(INamespaceContext **aCtx)
{
    if (gSF.gClassBuilder == NULL)
    {
        gSF.gClassBuilder = (*aCtx)->GetInstanceByClassHandle(aCtx, gSF.iClassBuilder);
    }
    return gSF.gClassBuilder->createInstance(ClassInfo(TCollection));
}

int _impl_elementCount(IInterface **aThis)
{
    struct TCollectionFields *vFields = gSF.gClassBuilder->getFields(aThis);
    return vFields->cursor;
}

int _impl_capacity(IInterface **aThis)
{
    return 10;
}

void _impl_push(IInterface **aThis, void *vObj)
{
    struct TCollectionFields *vFields = gSF.gClassBuilder->getFields(aThis);
    if (vFields->cursor < 10)
    {
        vFields->buffer[vFields->cursor++] = vObj;
    }
    return;
}

void *_impl_pop(IInterface **aThis)
{
    struct TCollectionFields *vFields = gSF.gClassBuilder->getFields(aThis);
    if (0 < vFields->cursor)
    {
        return vFields->buffer[--vFields->cursor];
    }
    return NULL;
}

void *_impl_icast(IInterface **aThis, TClassHandle aClassHandle)
{
    return gSF.gClassBuilder->getInterface(aThis, aClassHandle);
}

BEGIN_CLASS_IMPLEMENTATION(TCollection)
{
    MAP_OBJ_CAST_CALL(_impl_cast);
    MAP_OBJ_DSTR_CALL(_impl_dstr);
    MAP_OBJ_CSTR_CALL(_impl_cstr);
    MAP_CALL(elementCount, _impl_elementCount);
    MAP_CALL(capacity, _impl_capacity);
}
MAP_INTERFACE(IStack)
{
    MAP_CAST_CALL(_impl_icast);
    MAP_CALL(push, _impl_push);
    MAP_CALL(pop, _impl_pop);
}
END_CLASS_IMPLEMENTATION

__declspec(dllexport) void Installer(TOpenNamespaceFn nsFn)
{
    INamespaceContext **vMyCtx;

    {
        vMyCtx = nsFn(CFABRIC_MOD_NAME);
        gSF.iClassBuilder = (*vMyCtx)->GetClassHandleByName(vMyCtx, "TClassBuilder");
    }

    {
        vMyCtx = nsFn("Collection");
        gSF.iStack = (*vMyCtx)->GetClassHandleByName(vMyCtx, "IStack");
    }

    ImplementClass(TCollection, vMyCtx, nsFn);
}
