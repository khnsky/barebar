PROGRAM  := barebar
CXX      ?= g++
CXXFLAGS += -std=c++17 -Wall -Wextra -Werror -g
LDFLAGS  += -lxcb -lxcb-randr -lxcb-ewmh -lxcb-icccm
SRC      := $(wildcard src/*.cpp)

all: $(PROGRAM)

$(PROGRAM): $(SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
