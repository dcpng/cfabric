#include <string.h>
#include <windows.h>
#include <stdio.h>

#define DoTest(name)                                                                   \
    gDllHandle = LoadLibrary(TEXT("../" #name "/.makefiles/test/" #name ".test.dll")); \
    TestMain = GetProcAddress(gDllHandle, "Run");                                      \
    TestMain();

void main(void)
{
    void (*TestMain)();
    HANDLE gDllHandle;
    DoTest(PluginSys);
    DoTest(TClassAction);
}