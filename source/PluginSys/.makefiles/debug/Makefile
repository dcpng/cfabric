GccPath = C:\TDM-GCC-32\bin
SrcPath = ../..

###################################################
# [Build Debug modules]
###################################################
.PHONY: build
build: clean PluginSys.debug.dll

PluginSys.debug.dll : PluginSys.debug.o
	$(GccPath)/g++ $^ -shared -o $@

PluginSys.debug.o:
	$(GccPath)/g++ -c -g $(SrcPath)/PluginSys.cpp -I$(SrcPath)/ -lstdc++ -o $@

.PHONY: clean
clean: 
	del *.o
	del *.dll
