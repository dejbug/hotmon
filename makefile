hotmon.dll: hotmon.o hotkey.o debug.o
	@g++ -o $@ $^ -shared

test.exe: test.o hotmon.dll
	@g++ -o $@ $^

test: hotmon.dll test.exe
	@.\test.exe

clean:
	@del *.exe *.o *.pyc 2>NUL

reset:
	@make clean 1>NUL
	@del *.dll 2>NUL
