CXX=g++
CXXFLAGS=-Wall -c -std=c++11 -DNOT_USE_BLOCKMGR -DSINGLE_PLANE -DBYPASS_BUS -DNO_NOOP -DNO_BLOCK_STATE -DNO_PLANE_STATE -DNO_PHYSICAL_READ_EVENTS
#CXXFLAGS=-Wall -c -std=c++11 -g -pg -DUSE_BLOCKMGR -DCHECK_VALID_PAGE -DCHECK_HOT_VALID_PAGES
#CXXFLAGS=-Wall -c -std=c++11 -g -pg -O2 -DUSE_BLOCKMGR -DNDEBUG
#CXXFLAGS=-Wall -c -std=c++11 -O3 -DNDEBUG -DUSE_BLOCKMGR
LDFLAGS=
HEADERS=ssd.h
SOURCES_SSDLIB = $(filter-out ssd_ftl.cpp, $(wildcard ssd_*.cpp))  \
                 $(wildcard FTLs/*.cpp)                            \
                 $(wildcard HCident/*.cpp)                         \
                 $(wildcard GCAs/*.cpp)                            \
                 $(wildcard WLvl/*.cpp)                            \
                 $(wildcard util/*.cpp)                            \
                 SSDSim.cpp
OBJECTS_SSDLIB=$(patsubst %.cpp,%.o,$(SOURCES_SSDLIB))
SOURCES_RUNS = $(wildcard run_Sim*.cpp) run_ComputeOracle.cpp run_ConvertBIOTrace.cpp
PROGRAMS = $(patsubst run_%.cpp,%,$(SOURCES_RUNS))


all: $(PROGRAMS)

debug: CXXFLAGS += -DDEBUG -g -pg -DCHECK_VALID_PAGES -DCHECK_HOT_VALID_PAGES
debug: LDFLAGS += -pg
debug: $(PROGRAMS)

dbg: CXXFLAGS += -g -pg -DNDEBUG
dbg: LDFLAGS += -pg
dbg: $(PROGRAMS)

profile: CXXFLAGS += -g -pg -O2 -DNDEBUG
profile: LDFLAGS += -pg
profile: $(PROGRAMS)

release: CXXFLAGS += -O3 -DNDEBUG
release: $(PROGRAMS)

fast: CXXFLAGS += -Ofast -DNDEBUG
fast: $(PROGRAMS)

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
