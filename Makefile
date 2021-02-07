PROGRAM := barebar
CXX     ?= g++
LDFLAGS += -lxcb -lxcb-randr -lxcb-ewmh
SRC     := $(wildcard src/*.cpp)

all: $(PROGRAM)

$(PROGRAM): $(SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
