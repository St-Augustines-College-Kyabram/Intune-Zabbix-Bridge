CXX ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Wpedantic -Werror -fPIC
LDFLAGS ?=
.PHONY: all test clean
all: libsummary_transport.so
libsummary_transport.so: src/summary_transport.cpp src/summary_transport.h
	$(CXX) $(CXXFLAGS) -shared src/summary_transport.cpp -Wl,-z,relro,-z,now $(LDFLAGS) -lz -o $@
test: libsummary_transport.so
	$(CXX) $(CXXFLAGS) tests/summary_transport_test.cpp src/summary_transport.cpp $(LDFLAGS) -lz -o summary_transport_test
	./summary_transport_test
clean:
	rm -f libsummary_transport.so summary_transport_test
