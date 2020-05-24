#include <stdio.h>
#include "TCollection.h"

int main(int argc, char *argv[])
{
    TCollectionRef vObject = CreateTCollection();
    char vOutputBuff[256];
    char *vOutput = vOutputBuff;
    int vResultFromRemove;

    for (int i = 0; i < 11; i++)
    {
        sprintf(vOutput, "return of vObject.methods->Add(&vObject, i) = %d", vObject.methods->Add(&vObject, i));
    }

    for (int i = 0; i < 11; i++)
    {
        sprintf(vOutput, "return of vObject.methods->Remove(&vObject, i) = %d", vObject.methods->Remove(&vObject, &vResultFromRemove));
    }

    return 0;
}