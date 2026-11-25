CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -g

.PHONY: test clean

build/test_occupancy: tests/test_occupancy.cpp occupancy.ino tests/mock/Arduino.h tests/mock/LiquidCrystal.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -Itests/mock -o $@ tests/test_occupancy.cpp

test: build/test_occupancy
	@./build/test_occupancy

clean:
	rm -rf build
