#include "_internal.h"

struct cfb_class_helper_sFields
{
};

static struct cfb_class_helper_s **classBuilderImpl;

BEGIN_CLASS_INTERFACES(cfb_class_helper_s)

END_CLASS

/*This works because the instance's vtable pointer always points to somewhere
in the class instance. Hence we can use the big-pointer paradigm to reserve a 
header section after each vtable to hint its offset location and use it to calculate
the address of the header*/

/*return the vtable pointer address of the object's instance*/
static void *get_interface(void **aThis, cfb_class_hnd_t vClassHandle)
{
    /*vtable header always located behind the address of the current vtable*/
    __auto_type vVtbHdr = (struct _classbuilder_TVTableHeader *)((*aThis) - sizeof(struct _classbuilder_TVTableHeader));
    /*point to the header section of the class instance*/

    __auto_type vCastmap = ((struct _classbuilder_TClassHeader *)((void *)vVtbHdr - vVtbHdr->hdrOffset))->ptrCastMap;

    __auto_type vMyOffset = vCastmap->entries[vVtbHdr->castMapEntryIdx].fieldOffset;
    if (vClassHandle == IObjectHandler)
    {
        return aThis + vMyOffset;
    }

    for (int i = 0; i < vCastmap->entryCount; i++)
    {
        struct _classbuilder_TTypeCastMapEntry *vEntry = &vCastmap->entries[i];
        if (vEntry->handle == vClassHandle)
        {
            return (void *)aThis + (vEntry->fieldOffset - vMyOffset);
        }
    }
    return NULL;
}

static void *get_obj_data(void **aThis)
{
    /*vtable header always located behind the address of the current vtable*/
    __auto_type vVtbHdr = &((struct _classbuilder_TVTableHeader *)(*aThis))[-1];
    /*point to the data section of the object's instance*/
    return (void *)aThis + vVtbHdr->instanceDataOffset;
}

static void *alloc_obj(struct _classbuilder_TClassHeader *aClassHdr)
{
    struct _classbuilder_TVTableRef *vTableRef = calloc(1, aClassHdr->instanceByteSize);
    for (
        int i = 0, j = aClassHdr->ptrCastMap->entryCount - 1;
        i < j;
        i++)
    {
        vTableRef[i].ptrVtable = aClassHdr->ptrCastMap->entries[i + 1].ptrVtable;
    }
    return vTableRef;
}

static void del_obj(void ***aObj)
{
    if (**aObj)
    {
        /*vtable header always located behind the address of the current vtable*/
        __auto_type vVtbHdr = (struct _classbuilder_TVTableHeader *)((**aObj) - sizeof(struct _classbuilder_TVTableHeader));
        /*point to the data section of the object's instance*/
        struct cfb_object_s *vIObject =
            (void *)vVtbHdr + vVtbHdr->hdrOffset + sizeof(struct _classbuilder_TClassHeader) + sizeof(struct _classbuilder_TVTableHeader);
        vIObject->on_del(*aObj);
        free(get_interface(*aObj, IObjectHandler));
        *aObj = NULL;
    }
}

static void *on_new(cfb_plugin_t **aCtx)
{
    return alloc_obj(ClassInfo(cfb_class_helper_s));
}

static void on_del(void **aThis)
{
}

BEGIN_CLASS_IMPLEMENTATION(cfb_class_helper_s)
{
    MAP_METHOD(get_obj_data);
    MAP_METHOD(alloc_obj);
    MAP_METHOD(get_interface);
    MAP_METHOD(del_obj);
}
END_CLASS_IMPLEMENTATION
