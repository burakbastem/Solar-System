LDLIBS = -lglut -lGLEW -lGL -lGLU
CXXINCS = -Iinclude

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = solar_system

.PHONY: clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	g++ -g -O2 $(CXXINCS) $(OBJECTS) $(LDLIBS) -o $@
	
%.o: %.cpp
	g++ -c -Wall $(CXXINCS) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(TARGET)
