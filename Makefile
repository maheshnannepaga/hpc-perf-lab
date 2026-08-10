CXX ?= g++
CPPFLAGS := -Iinclude
CXXFLAGS ?= -O3 -std=c++20 -Wall -Wextra -Wpedantic
LDFLAGS :=

ifeq ($(OPENMP),1)
  CXXFLAGS += -fopenmp -DHPC_PERF_USE_OPENMP
endif

TARGET := bin/gemm_bench
SOURCES := src/gemm_bench.cpp

.PHONY: all clean test format

all: $(TARGET)

$(TARGET): $(SOURCES) | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

bin:
	mkdir -p $@

test: $(TARGET)
	./tests/smoke_test.sh $(TARGET)

clean:
	rm -rf bin build

