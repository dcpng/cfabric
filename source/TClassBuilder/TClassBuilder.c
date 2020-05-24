#include "TClassBuilder.h"

#define CastToParent(this) \
    (*((void **)(((void *)this) + sizeof(void *))))

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

/*This works because the instance's memory layout it structured such that
offset value is stored on the instance object just before the *This pointer*/
void *CastInterface(IInterface **aThis, TClassHandle vClassHandle)
{
    aThis = (IInterface **)CastToParent(aThis);
    //return (*aThis)->Cast(aThis, vClassHandle);
    return DoCast(aThis, vClassHandle);
}

extern TClassBuilder classBuilderImpl;

void *
_impl_constructor(INamespaceContext **aCtx)
{
    return &classBuilderImpl;
}

TClassBuilder classBuilderImpl = {
    .base = {
        .Constructor = _impl_constructor,
    },
    .getFields = GetInstanceData,
    .createInstance = CreateInstance,
    .getInterface = CastInterface,
    .rootCastOp = DoCast,
};

__declspec(dllexport) void Installer(TOpenNamespaceFn nsFn)
{
    __auto_type vCtx = nsFn(CFABRIC_MOD_NAME);
    (*vCtx)->MapClassByName(vCtx, &classBuilderImpl, "TClassBuilder");
}