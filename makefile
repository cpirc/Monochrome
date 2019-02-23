CXX = g++

FLAGS = -pthread -std=c++11 -Wall -Wextra -pipe
RELEASE_FLAGS = $(FLAGS) -O3 -flto -DNDEBUG
DEBUG_FLAGS = $(FLAGS) -fno-omit-frame-pointer -g

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
HEADERS = $(wildcard $(SRCDIR)/*.h)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

LINKS = -pthread -Wl,--no-as-needed

BIN = monochrome

ifeq ($(OS),Windows_NT)
    $(shell mkdir obj bin)
else
    $(shell mkdir -p obj bin)
endif

all: $(OBJECTS)
	$(CXX) $(FLAGS) $^ -o $(BINDIR)/$(BIN) $(LINKS)

release:
	$(MAKE) FLAGS="$(RELEASE_FLAGS)"

debug:
	$(MAKE) FLAGS="$(DEBUG_FLAGS)"

testing:
	$(MAKE) FLAGS="$(FLAGS) -DTESTING"

$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	-rm -rf $(OBJDIR) $(BINDIR)
