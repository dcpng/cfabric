GccPath = C:\TDM-GCC-32\bin
SrcPath = ..

.PHONY: build-test

build-test: Clean-up subsystems Test.exe

# Ensure all test modules are built
subsystems:
	$(MAKE) -C $(SrcPath)/PluginSys/.makefiles/test
	$(MAKE) -C $(SrcPath)/TClassBuilder/.makefiles/test

# Build the test harness
Test.exe: Test.o
	$(GccPath)\gcc -g  $^ -o $@

Test.o:
	$(GccPath)\gcc -c -g root-test.c -o $@

.PHONY: Clean-up

Clean-up:
	del *.o
	del *.dll
	del *.exe