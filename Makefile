CXX      = g++
CXXFLAGS = -std=c++17 -O2
TARGET   = task2
SRC      = TestWork2.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
