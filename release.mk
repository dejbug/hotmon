#$(eval CPPFLAGS:=-g) 
CPPFLAGS:=-DNDEBUG -Wall -std=c++98 -pedantic -Wextra -Wconversion
DEBUG_OBJ:=debug.o

hotmon.dll: hotmon.o hotkey.o $(DEBUG_OBJ)
	@g++ -o $@ $^ -shared
	
test.exe: test.o hotmon.dll
	@g++ -o $@ $^ -g

dist: hotmon.dll
	7z a -tzip hotmon.zip hotmon.dll hotmon.py vkdefs.py >NUL
