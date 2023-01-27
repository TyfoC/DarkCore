CXX=g++

CXXFLAGS=-std=c++17 -Wall -Wextra -I../include
LDFLAGS=-Wall -Wextra -lgcc

CXXSOURCES=$(shell find ../source -name "*.cxx")
CXXOBJECTS=$(addsuffix .o, $(CXXSOURCES))

.PHONY:	clean

lite-fs:
	$(MAKE) $(CXXOBJECTS)
	$(CXX) $(CXXOBJECTS) $(LDFLAGS) -o $@.elf

%.cxx.o:	%.cxx
	$(CXX) -c $< $(CXXFLAGS) -o $@

clean:
	rm -rf $(CXXOBJECTS) *.elf