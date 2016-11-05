DESTDIR?=""

INCLUDE= `sdl-config --cflags` `pkg-config --cflags gstreamer-1.0 gstreamer-plugins-base-1.0`
LIBS= `sdl-config --libs` -lSDL_gfx `pkg-config --libs gstreamer-1.0 gstreamer-plugins-base-1.0` -lgstapp-1.0 -lpthread

RPMRevolutionMeter_OBJS= main.o
RPMRevolutionMeter_LIBS= $(LIBS)

unittest_OBJS= \
	unittests/test.o \
	unittests/CappedStorageWaveform_Test.o \
	unittests/MinMaxCheck_Test.o \
	unittests/SlidingAverager_Test.o
unittest_LIBS= $(LIBS) -lboost_unit_test_framework

EXECS= RPMRevolutionMeter unittest
EXEC_installed= RPMRevolutionMeter

COMPILER_FLAGS+= -Wall -O3 -std=c++0x -ggdb

RPMRevolutionMeter: $(RPMRevolutionMeter_OBJS) $(wildcard *.h) $(wildcard *.hpp) Makefile
	$(CXX) $(COMPILER_FLAGS) -o $@ $($@_OBJS) $($@_LIBS)

unittest: $(unittest_OBJS) $(wildcard *.h) $(wildcard *.hpp) Makefile
	$(CXX) $(COMPILER_FLAGS) -o $@ $($@_OBJS) $($@_LIBS) 

%.o:	%.cpp
	$(CXX) -c $(COMPILER_FLAGS) -o $@ $< $(INCLUDE)

all: RPMRevolutionMeter unittest


.PHONY: test
test: unittest
	./unittest

.PHONY: install
install: $(EXEC_installed)
	install $(EXEC_installed) $(DESTDIR)/usr/local/bin

.PHONY: uninstall
uninstall:
	rm $(DESTDIR)/usr/local/bin/$(EXEC_installed)

.PHONY: prepare
prepare:
	apt-get install libsdl1.2-dev libsdl-gfx1.2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

.PHONY: prepare-all
prepare-all: prepare
	apt-get install libboost-test-dev


.PHONY: clean
clean:
	rm -f $(EXECS) $(RPMRevolutionMeter_OBJS) $(unittest_OBJS)
