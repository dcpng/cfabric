#ifndef ISTACK_H_
#define ISTACK_H_

#include "../../../PluginSys/PluginSys.h"
#include "../../TClassBuilder.h"

struct IStack
{
    DECLARE_INTERFACE;
    void (*push)(IInterface **aThis, void *data);
    void *(*pop)(IInterface **aThis);
};

#endif