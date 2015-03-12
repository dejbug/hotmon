include debug.mk

clean:
	@del *.o *.pyc test.exe 2>NUL

clobber:
	@make clean 1>NUL
	@del hotmon.dll hotmon.zip 2>NUL
