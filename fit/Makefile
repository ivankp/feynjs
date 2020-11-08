.PHONY: all clean

CPPFLAGS := -std=c++20 -Iinclude
CXXFLAGS := -Wall -O3 -flto -fmax-errors=3
# CXXFLAGS := -Wall -O0 -g -fmax-errors=3

EXE := fit

L_fit := -lfmt

all: $(EXE)

fit: linalg.hh linalg.cc wls.hh wls.cc

%: %.cc
	g++ $(CPPFLAGS) $(CXXFLAGS) -o $@ $(filter %.cc,$^) $(L_$*)

clean:
	@rm -fv $(EXE)

