SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.o,$(SRC))
TARGET=libfuture.so
CPPFLAGS= -g -std=c++20 -fcoroutines -fPIC
LIBS= -lpthread
$(TARGET):$(OBJ)
	g++ -shared $+ -o $@ $(LIBS) $(CPPFLAGS)

clean:
	rm $(OBJ) $(TARGET)

