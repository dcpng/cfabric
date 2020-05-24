GccPath = C:\TDM-GCC-32\bin
SrcPath = ../..

###################################################
# [Build Debug modules]
###################################################
.PHONY: build
build: TClassBuilder.debug.dll

TClassBuilder.debug.dll : TClassBuilder.debug.o
	$(GccPath)/g++ $^ -shared -o $@

TClassBuilder.debug.o:
	$(GccPath)/gcc -c -g $(SrcPath)/TClassBuilder.c -I$(SrcPath)/ -o $@
