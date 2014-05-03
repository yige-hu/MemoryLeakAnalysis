all: SamplePass.so FunctionInfo.so MemLeak.so

ANDERS_DIR=../andersen
ANDERS_INCLUDES=$(ANDERS_DIR)/include
ANDERS_LIB=$(ANDERS_DIR)/Debug+Asserts/lib

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -std=c++11

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -I$(ANDERS_INCLUDES) -o $@ $^

%.so: %.o
	$(CXX) $(CXXFLAGS) -shared -L$(ANDERS_LIB) -lAnders -o $@ $^     # -flat_namespace 
clean:
	rm -f *.o *~ *.so
