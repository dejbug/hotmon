#include debug.mk
MODE:=debug

.PHONY: debug
debug:
	$(eval MODE:=debug)
	@echo (building $(MODE).mk)
#	@make -f $(MODE).mk 1>NUL

.PHONY: release
release:
	$(eval MODE:=release)
	@echo (building $(MODE).mk)
#	@make -f $(MODE).mk 1>NUL

%:
	@make -f $(MODE).mk $@ 1>NUL

clean:
	@del *.o *.pyc test.exe 2>NUL

clobber:
	@make clean 1>NUL
	@del hotmon.dll hotmon.zip 2>NUL
