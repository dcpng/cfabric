#include <string.h>
#include <windows.h>
#include <stdio.h>
#include "TCollection.h"
#include "IStack.h"
#include <unit-test/bdd-for-c.h>

__declspec(dllexport) void Run()
{
    main();
}

spec("TClassBuilder.dll")
{

    static HANDLE vPluginSysDll;
    static HANDLE vTClassBuiderDll;
    static HANDLE vTCollectionDll;
    static TOpenNamespaceFn vNs;
    static TModInstaller vInstaller;
    static INamespaceContext **vCtx;
    static TClassHandle TCollectionHandler;
    static TClassHandle IStackHandler;
    static struct TCollection **vObj;
    static struct IStack **vIStackObj;
    static int vData[] = {1, 2, 3, 4, 5};
    before()
    {
        /*open cfabric plugin loader*/
        vPluginSysDll = LoadLibrary(TEXT("../PluginSys/.makefiles/debug/PluginSys.debug.dll"));
        vNs = (TOpenNamespaceFn)GetProcAddress(vPluginSysDll, "OpenNamespace");

        /*Load class module*/
        vTClassBuiderDll = LoadLibrary(TEXT("../TClassBuilder/.makefiles/debug/TClassBuilder.debug.dll"));
        vInstaller = (TModInstaller)GetProcAddress(vTClassBuiderDll, "Installer");
        vInstaller(vNs);

        vTCollectionDll = LoadLibrary(TEXT("../TClassBuilder/.makefiles/test/TCollection.debug.dll"));
        vInstaller = (TModInstaller)GetProcAddress(vTCollectionDll, "Installer");
        vInstaller(vNs);
    }

    it("Testing GetInstanceByClassHandle method")
    {
        vCtx = vNs("Collection");
        TCollectionHandler = (*vCtx)->GetClassHandleByName(vCtx, "TCollection");
        vObj = (*vCtx)->GetInstanceByClassHandle(vCtx, TCollectionHandler);
        check((vObj != NULL));
    }

    it("Testing rootCastOp and getFields though instance method call that read a value of a field")
    {
        check((*vObj)->elementCount(vObj) == 0);
    }

    it("Testing getInterface though instance method call that read a value of a field")
    {
        IStackHandler = (*vCtx)->GetClassHandleByName(vCtx, "IStack");
        vIStackObj = (*vObj)->base.baseClass.Cast(vObj, IStackHandler);

        for (int i = 0; i < 5; i++)
        {
            (*vIStackObj)->push(vIStackObj, &vData[i]);
        }

        for (int i = 5, j; i > 0; i--)
        {
            j = *((int *)(*vIStackObj)->pop(vIStackObj));
            check(j == i);
        };
    }
}