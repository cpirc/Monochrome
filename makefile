CXX = g++

CXX_FLAGS = -std=c++11 -Wall -Wextra -O3 -flto -pipe

CXX_FILES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)

OBJS = $(patsubst %.cpp,%.o,$(CXX_FILES))
LINKS =

EXEC = engine

all: $(OBJS)
	$(CXX) $(CXX_FLAGS) $(OBJS) -o $(EXEC) $(LINKS)

$(OBJS): $(CXX_FILES) $(HEADERS)
	$(CXX) $(CXX_FLAGS) -c $(CXX_FILES)
