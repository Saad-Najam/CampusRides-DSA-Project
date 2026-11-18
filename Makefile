# Campus Rides - build with:  make        (or mingw32-make on Windows)
#   make run    build + start the app          make test   build + run the test driver
#   make seed   start with demo data            make clean
CXX      ?= g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Isrc

COMMON  := $(wildcard src/common/*.cpp)
M1      := $(filter-out src/member1_saad/main.cpp,$(wildcard src/member1_saad/*.cpp))
M2      := $(wildcard src/member2_ayaan/*.cpp)
M3      := $(wildcard src/member3_shamaem/*.cpp)
LIB_SRC := $(COMMON) $(M1) $(M2) $(M3)
LIB_OBJ := $(LIB_SRC:src/%.cpp=build/%.o)

TESTS   := $(wildcard tests/*.cpp)
TEST_OBJ := $(TESTS:tests/%.cpp=build/tests/%.o)

all: campusrides

campusrides: $(LIB_OBJ) build/member1_saad/main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

test_runner: $(LIB_OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

build/tests/%.o: tests/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

run: campusrides
	./campusrides

seed: campusrides
	./campusrides --seed

test: test_runner
	./test_runner

clean:
	rm -rf build campusrides campusrides.exe test_runner test_runner.exe test_data*

-include $(LIB_OBJ:.o=.d) $(TEST_OBJ:.o=.d) build/member1_saad/main.d

.PHONY: all run seed test clean
