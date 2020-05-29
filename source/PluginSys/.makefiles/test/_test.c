#include <string.h>
#include <windows.h>
#include <stdio.h>
#include "TApple.h"

HANDLE vPluginSysHandle, vClassActionHandle;
TModInstaller vInstaller;
TOpenNamespaceFn vNamespace;
INamespaceContext **vCtx, **vCtx2;
TClassHandle vClsHandle1, vClsHandle2, hApple, hCollection;
struct TClassAction **vClassAction;
struct TApple **vInstance;
struct IGoodBye **vGoodBye;
struct IHello **vHello;

#include <unit-test/bdd-for-c.h>

__declspec(dllexport) void Run()
{
    main();
}

spec("PluginSys.dll")
{

    it("should load the Plugin Context")
    {
        vPluginSysHandle = LoadLibrary(TEXT("../PluginSys/.makefiles/debug/PluginSys.debug.dll"));
        check(vPluginSysHandle != 0);

        vClassActionHandle = LoadLibrary(TEXT("../TClassAction/.makefiles/debug/TClassAction.debug.dll"));
        check(vClassActionHandle != 0);
    }

    it("should load the CreateContext and install the TClassAction.dll plugin")
    {
        vNamespace = (TOpenNamespaceFn)GetProcAddress(vPluginSysHandle, "OpenNamespace");
        check(vNamespace != 0);

        __auto_type vInstaller = (TModInstaller)GetProcAddress(vClassActionHandle, "Installer");
        vInstaller(vNamespace);
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
        check(vClsHandle1 == vClsHandle2);
    }

    it("handlers are DIFFERENT from GetClassHandleByName when the classnames are DIFFERENT")
    {
        vClsHandle1 = (*vCtx)->GetClassHandleByName(vCtx, "ICLASS1");
        vClsHandle2 = (*vCtx)->GetClassHandleByName(vCtx, "ICLASS2");
        check(vClsHandle1 != vClsHandle2);
    }

    it("Installing Plugin class")
    {
        vClassAction = InstallTApple(vNamespace);
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
        vHello = (*vClassAction)->typeCast(vInstance, hHello);
        check(vHello != NULL);
        vGoodBye = (*vClassAction)->typeCast(vInstance, hGoodBye);
        check(vGoodBye != NULL);

        /*test casting from InterfaceA <-> InterfaceB*/
        void *vPtr = (*vClassAction)->typeCast(vHello, hGoodBye);
        check(vPtr == vGoodBye);

        vPtr = (*vClassAction)->typeCast(vGoodBye, hHello);
        check(vPtr == vHello);

        /*test casting from InterfaceA -> InterfaceA*/
        vPtr = (*vClassAction)->typeCast(vGoodBye, hGoodBye);
        check(vPtr == vGoodBye);

        vPtr = (*vClassAction)->typeCast(vHello, hHello);
        check(vPtr == vHello);

        /*test casting from Interface -> base*/
        vPtr = (*vClassAction)->typeCast(vHello, hApple);
        check(vPtr == vInstance);

        vPtr = (*vClassAction)->typeCast(vGoodBye, hApple);
        check(vPtr == vInstance);
    }

    it("Test: method call")
    {
        char vResult[64];
        (*vHello)->Say(vHello, vResult);
        check(strncmp(vResult, "data = Hello", 64) == 0);

        (*vGoodBye)->Say(vGoodBye, vResult);
        check(strncmp(vResult, "data = Good Bye", 64) == 0);
    }
}