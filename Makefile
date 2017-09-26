CXX=g++
#CXXFLAGS=-Wall -c -std=c++11  -g -pg -DUSE_BLOCKMGR -DCHECK_VALID_PAGE -DCHECK_HOT_VALID_PAGES
#CXXFLAGS=-Wall -c -std=c++11 -g -pg -O2 -DUSE_BLOCKMGR
#LDFLAGS= -pg
CXXFLAGS=-Wall -c -std=c++11 -O3 -DNDEBUG -DUSE_BLOCKMGR
LDFLAGS=
HEADERS=ssd.h
SOURCES_SSDLIB = $(filter-out ssd_ftl.cpp, $(wildcard ssd_*.cpp))  \
                 $(wildcard FTLs/*.cpp)                            \
                 $(wildcard HCident/*.cpp)                         \
                 $(wildcard GCAs/*.cpp)                            \
                 $(wildcard util/*.cpp)                            \
                 SSDSim.cpp
OBJECTS_SSDLIB=$(patsubst %.cpp,%.o,$(SOURCES_SSDLIB))
SOURCES_RUNS = $(wildcard run_Sim*.cpp) run_ComputeOracle.cpp
PROGRAMS = $(patsubst run_%.cpp,%,$(SOURCES_RUNS))


all: $(PROGRAMS)

.cpp.o: $(HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@

define PROGRAM_TEMPLATE
  $1 : run_$1.o $$(OBJECTS_SSDLIB)
	$$(CXX) $$(LDFLAGS) $$< $$(OBJECTS_SSDLIB) -o $1.exe ${@:2}
endef

$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_TEMPLATE,$(prog))))

clean:
	-rm -rf *.o FTLs/*.o GCAs/*.o util/*.o $(PROGRAMS) *.exe

.PHONY: files
files:
	@echo $(SOURCES_SSDLIB) $(SOURCES_RUNS) $(HEADERS) | tr ' ' '\n'
