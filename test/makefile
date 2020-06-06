srcDir = ../source


.PHONY: build-test
# Build the test harness
build-test: clean-up Test.exe

Test.exe : Test.o core.o cfb_class_helper.o module.o
	gcc -g  $^ -o $@ -lstdc++

Test.o:
	gcc -c -g main.c -o $@

module.o: $(srcDir)/module.c
	gcc -c -g $^ -o $@

core.o: $(srcDir)/core.cpp
	g++ -c -g $^ -o $@

cfb_class_helper.o: $(srcDir)/cfb_class_helper.c
	gcc -c -g $^ -o $@

.PHONY: clean-up
clean-up:
	del *.o
	del *.a
	del *.dll
	del *.exe