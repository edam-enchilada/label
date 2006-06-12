include ../makefiles/make.header

UB_ROOT = $(SPECTRUM_ROOT)

PURE_OPTS = -best-effort -g++=yes -windows=no -copy_fd_output_to_logfile=1 -log_file=pure.log -cache-dir=/tmp -always-use-cache-dir=yes

MAIN_OBJ = main.o

INCLUDES = -I$(UB_ROOT)/include

LIB = datagen dataio label datatype Clp Coin

LDFLAGS = -L$(UB_ROOT)/lib $(addprefix -l, ${LIB})

LDFLAGS += -L$(UB_ROOT)/COIN/lib -lClp -lCoin

CXXFLAGS += $(INCLUDES)

.PHONY : all clean

all : spectrum

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

spectrum : setup $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -g -o $@ $(MAIN_OBJ) $(LDFLAGS)
	rm -f $(UB_ROOT)/bin/spectrum
	ln -s $(UB_ROOT)/spectrum/spectrum $(UB_ROOT)/bin/spectrum

cleanAnalyser : 
	rm -rf analyser

analyser : analyser.C cleanAnalyser
	$(CXX) $(CXXFLAGS) $(INCLUDE) -g -o analyser analyser.C ../COIN/lib/libCoin.so ../COIN/lib/libClp.so $(LDFLAGS)

cleanConverter :
	rm -rf converter 

converter : converterMain.C cleanConverter	
	$(CXX) $(CXXFLAGS) $(INCLUDE) -g -o converter converterMain.C ../COIN/lib/libCoin.so \
	../COIN/lib/libClp.so $(LDFLAGS)

cleanDataGen :
	rm -rf datagen
	rm -rf datagen2

datagen : cleanDataGen
	$(CXX) $(CXXFLAGS) $(INCLUDE) -g -o datagen datagenMain.C ../COIN/lib/libCoin.so \
	../COIN/lib/libClp.so $(LDFLAGS)

datagen2 : cleanDataGen
	$(CXX) $(CXXFLAGS) $(INCLUDE) -g -o datagen2 datagenMain2.C ../COIN/lib/libCoin.so \
	../COIN/lib/libClp.so $(LDFLAGS)

setup :
	rm -rf main.o

pspectrum : setup $(MAIN_OBJ)
	purify $(PURE_OPTS) $(CXX) $(CXXFLAGS) -g  $(MAIN_OBJ) $(LDFLAGS) -o $@

clean :
	rm -f spectrum *.o *~ core

depend :

# DO NOT DELETE
