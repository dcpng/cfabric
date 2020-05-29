###################################################
# [Build Test modules]
###################################################

GccPath = C:\TDM-GCC-32\bin
BuildPath = ..
SrcPath = $(BuildPath)/..
DebugObjPath = $(BuildPath)/debug

.PHONY: build
build: Clean-up subsystems PluginSys.test.dll

## Build all necessary dependent object files first
subsystems:
	$(MAKE) -C $(DebugObjPath)

PluginSys.test.dll : PluginSys.test.o TApple.o
	$(GccPath)\gcc $^ -shared -o $@

PluginSys.test.o:
	$(GccPath)\gcc -c -g _test.c -I$(SrcPath) -o $@

TApple.o:
	$(GccPath)\gcc -c -g TApple.c -I$(SrcPath) -o $@

.PHONY: Clean-up
Clean-up:
	del *.o
	del *.dll