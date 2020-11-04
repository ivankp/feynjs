CPPFLAGS := -std=c++20 -Iinclude
CXXFLAGS := -Wall -O3 -flto -fmax-errors=3
# CXXFLAGS := -Wall -O0 -g -fmax-errors=3

all: fit

fit: %: %.cc
	g++ $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

