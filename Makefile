CC = g++
CFLAGS = -Wall -g -std=c++0x
LFLAGS = -L ~/local/lib
INCLUDES = -I ~/local/include

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)
TEST_SRCS := $(wildcard test/*.cpp)
TEST_OBJS := $(TEST_SRCS:.cpp=.o)

OPENCVLIB = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio
LOCALLIB= ~/local/lib
MOMOLIBS= -lmomoLogging 

all: test/testRunner service/service

install: $(LOCALLIB)/libmomoImg.a

$(LOCALLIB)/libmomoImg.a : $(OBJS)
	cp src/*.hpp ~/local/include/momo
	ar rvs ~/local/lib/libmomoImg.a src/*.o

test/testRunner: $(TEST_OBJS) install
	$(CC) $(LFLAGS) -o test/testRunner $(TEST_OBJS) $(MOMOLIBS) -lmomoImg $(OPENCVLIB)

test/%.o: test/%.cpp test/%.hpp $(LOCALLIB)/libmomoMath.a
	$(CC) -c $(INCLUDES) $(CFLAGS) $<
	mv *.o test

src/%.o: src/%.cpp src/*.hpp
	$(CC) -c $(INCLUDES) $(CFLAGS) $<
	mv *.o src

