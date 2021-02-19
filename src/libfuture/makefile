SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.o,$(SRC))
TARGET=test_future
CPPFLAGS= -g -std=c++20 -fcoroutines
LIBS= -lpthread
$(TARGET):$(OBJ)
	g++ $+ -o $@ $(LIBS) $(CPPFLAGS)

clean:
	rm $(OBJ) $(TARGET)

