# Makefile

.PHONY: setup install clean

setup install:
	find $(SPECTRUM_ROOT) -name '*.h'  -exec ln -s {} . \;	

clean :
	rm -f *.hpp
	rm -f *.h
