CXX = mpic++
CXXFLAGS = -std=c++0x -Wall -Wextra -Wshadow -Werror -O3 -DNDEBUG

INCLUDES =
LDFLAGS =
LIBS =

SOURCE = src/cg
TARGET = cg
OBJS = $(TARGET).o

all: $(TARGET)

$(TARGET): $(OBJS) Makefile
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

$(TARGET).o: $(SOURCE).cpp src/matrix.h src/matrix.cpp src/Timer.h Makefile 
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $(SOURCE).cpp

clean:
	@$(RM) -rf *.o $(TARGET)
