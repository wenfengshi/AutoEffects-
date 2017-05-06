CXX=g++ -std=c++11
CXXFLAGS:=-I../../include
CXXFLAGS+=$(shell pkg-config --cflags opencv)

LDFLAGS+=-L../../libs/linux-x86_64/lib_dummy

LIBS:=-lcvface_api -lpthread
LIBS+=$(shell pkg-config --libs opencv)

SRCS:=$(shell echo *.cpp)
TARGETS:=$(patsubst %.cpp, test_%, $(SRCS))

all: $(TARGETS)
	chmod +x *.sh

test_%: %.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -f *.o test_*
