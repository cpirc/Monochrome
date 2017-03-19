CXX = g++

FLAGS = -std=c++14 -Wall -Wextra -pipe
RELEASE_FLAGS = $(FLAGS) -O3 -flto -DNDEBUG
DEBUG_FLAGS = $(FLAGS) -fno-omit-frame-pointer -g

CXX_FILES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)

OBJS = $(patsubst %.cpp,%.o,$(CXX_FILES))
LINKS =

EXEC = engine

all: $(OBJS)
	$(CXX) $(FLAGS) $^ -o $(EXEC) $(LINKS)

release:
	$(MAKE) FLAGS="$(RELEASE_FLAGS)"

debug:
	$(MAKE) FLAGS="$(DEBUG_FLAGS)"

%.o: %.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	-rm -f $(OBJS)
