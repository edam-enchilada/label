# Makefile

.PHONY: setup install clean

setup install: clean
	find $(SPECTRUM_ROOT) -name '*.h' | sort -u | gawk '{system("ln -s " $$1 " .")}'

clean :
	rm -f *.hpp
	rm -f *.h
