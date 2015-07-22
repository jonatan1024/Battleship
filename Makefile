CXXFLAGS=-Wall -pedantic -Wno-long-long -O0 -ggdb -Wextra -std=c++11
LDFLAGS=-lncurses
EXECUTABLE=matejjon
FILES=AI Client Field Game Grid matejjon Menu Server Ship Socket
SOURCES=$(addprefix src/, $(addsuffix .cpp, $(FILES)))
DOXYFILE=src/Doxyfile

default: all

all: compile doc

compile: $(SOURCES:.cpp=.o)
	$(CXX) $(SOURCES:.cpp=.o) -o $(EXECUTABLE)  $(LDFLAGS)

run:
	./$(EXECUTABLE)

clean:
	$(RM) $(SOURCES:.cpp=.o) $(EXECUTABLE)
	-rm -rf doc

doc:
	-doxygen $(DOXYFILE)
