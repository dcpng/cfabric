#ifndef TCOLLECTION_H
#define TCOLLECTION_H

#include "../../../PluginSys/PluginSys.h"
#include "../../TClassBuilder.h"

struct TCollection
{
    DECLARE_CLASS;
    int (*elementCount)(IInterface **athis);
    int (*capacity)(IInterface **athis);
};

#endif