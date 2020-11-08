.PHONY: all clean

CPPFLAGS := -std=c++20 -Iinclude
CXXFLAGS := -Wall -O3 -flto -fmax-errors=3
# CXXFLAGS := -Wall -O0 -g -fmax-errors=3

EXE := fit

L_fit := -lgsl -lgslcblas -lfmt

all: $(EXE)

%: %.cc
	g++ $(CPPFLAGS) $(CXXFLAGS) -o $@ $< $(L_$*)

clean:
	@rm -fv $(EXE)

