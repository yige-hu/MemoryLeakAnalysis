all: samplePass memLeak functionInfo

samplePass: SamplePass.so
memLeak: MemLeak.so
functionInfo: FunctionInfo.so

ANDERS_DIR=../andersen
ANDERS_INCLUDES=$(ANDERS_DIR)/include
ANDERS_LIB=$(ANDERS_DIR)/Debug+Asserts/lib

CXXFLAGS_STATIC = $(shell llvm-config --cxxflags) -g -O0 -std=c++11
CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -std=c++11

MEMLEAK_OBJ= MemLeak.o HelperFunction.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -I$(ANDERS_INCLUDES) -o $@ $^

%.so: %.o
	$(CXX) $(CXXFLAGS) -shared -L$(ANDERS_LIB) -lAnders -o $@ $^

#HelperFunction.o: HelperFunction.cpp
#	$(CXX) $(CXXFLAGS) -c -I$(ANDERS_INCLUDES) -o $@ $^

#MemLeak.so: $(MEMLEAK_OBJ)
#	$(CXX) -fPIC $(CXXFLAGS_STATIC) -shared -L$(ANDERS_LIB) -lAnders -o $@ $(MEMLEAK_OBJ)

clean:
	rm -f *.o *~ *.so *.bc *.ll
