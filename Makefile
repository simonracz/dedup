SRCDIR := $(CURDIR)/src
BUILDDIR := $(CURDIR)/build
BINARY := dedup

.PHONY: all debug release clean

all: debug

FLAGS = -MMD -MP

debug: FLAGS += -DDEBUG -g -Og
debug: $(BINARY)

release: FLAGS += -O2 -march=native -mtune=native
release: $(BINARY)

SOURCES := $(shell find $(SRCDIR) -name \*.cpp)
HEADERS := $(shell find $(SRCDIR) -name \*.h)
DEPENDS := $(subst $(SRCDIR),$(BUILDDIR),$(SOURCES:.cpp=.d))
OBJECTS := $(subst $(SRCDIR),$(BUILDDIR),$(SOURCES:.cpp=.o))
DIRS := $(sort $(dir $(SOURCES)))
OBJDIRS := $(subst $(SRCDIR),$(BUILDDIR),$(DIRS))
INCLUDES := $(foreach dir, $(dir $(HEADERS)), $(addprefix -I, $(dir)))
VPATH := $(DIRS)

CFLAGS = $(FLAGS) -Wall -Wextra -std=c17
CXXFLAGS = $(FLAGS) -Wall -Wextra -std=c++17
#LDFLAGS :=
#LDLIBS := -lm

-include $(DEPENDS)

$(BINARY): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJECTS): $(BUILDDIR)/%o: $(SRCDIR)/%cpp Makefile | $(BUILDDIR) $(OBJDIRS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(OBJDIRS):
	mkdir -p $@

clean:
	@$(RM) $(OBJECTS) $(BINARY) $(DEPENDS)
	@$(RM) -rd $(BUILDDIR)
