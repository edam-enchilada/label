
export SPECTRUM_ROOT := ..

.PHONY: spectrum all clean setup install depend makeClp cleanClp

SUBDIRS = label dataio datatype tests datagen spectrum

spectrum: setup
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) all); done;

depend :
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) depend); done;

clean:
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean); done;

setup:
	cd include; $(MAKE) $@

clp :
	cd COIN/Coin;make
	cd COIN/Clp;make

clean_clp :
	cd COIN/Coin;make clean
	cd COIN/Clp;make clean

clean_all : clean_clp clean

all : clp spectrum
