CXX = g++

FLAGS = -pthread -std=c++11 -Wall -Wextra -pipe
RELEASE_FLAGS = $(FLAGS) -O3 -flto -DNDEBUG
DEBUG_FLAGS = $(FLAGS) -fno-omit-frame-pointer -g

CXX_FILES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)

OBJS = $(patsubst %.cpp,%.o,$(CXX_FILES))
LINKS = -pthread -Wl,--no-as-needed

EXEC = monochrome

all: $(OBJS)
	$(CXX) $(FLAGS) $^ -o $(EXEC) $(LINKS)

release:
	$(MAKE) FLAGS="$(RELEASE_FLAGS)"

debug:
	$(MAKE) FLAGS="$(DEBUG_FLAGS)"

testing:
	$(MAKE) FLAGS="$(FLAGS) -DTESTING"

%.o: %.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	-rm -f $(OBJS) $(EXEC)
