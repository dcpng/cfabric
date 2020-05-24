###################################################
# [Build Test modules]
###################################################

GccPath = C:\TDM-GCC-32\bin
BuildPath = ..
SrcPath = $(BuildPath)/..
DebugObjPath = $(BuildPath)/debug

.PHONY: build
build: Clean-up subsystems TCollection.debug.dll TClassBuilder.test.dll

## Build all necessary dependent object files first
subsystems:
	$(MAKE) -C $(DebugObjPath)

TClassBuilder.test.dll : TClassBuilder.test.o
	$(GccPath)/gcc $^ -lstdc++ -shared -o $@

TClassBuilder.test.o:
	$(GccPath)/gcc -c -g _test.c -I$(SrcPath) -o $@

TCollection.debug.dll : TCollection.debug.o
	$(GccPath)/gcc $^ -shared -o $@

TCollection.debug.o:
	$(GccPath)/gcc -c -g TCollection.c -o $@

.PHONY: Clean-up
Clean-up:
	del *.o
	del *.dll