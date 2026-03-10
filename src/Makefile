ARCH        := $(shell uname -m)
CC          := g++ -std=c++17 -Wno-psabi
SRCEXT      := cc

CHIPDIR     := /usr/local/src/Chipmunk2D
ENVIRODIR   := /usr/local/src/enviro/server/include

CFLAGS      := -ggdb -shared -fPIC
INCLUDE     := -I $(ENVIRODIR) -I $(CHIPDIR)/include/chipmunk

TARGETDIR   := ../lib
SOURCES     := $(wildcard *.cc)
TARGETS     := $(patsubst %.cc,%.so,$(wildcard *.cc))
FULL_TARGETS := $(addprefix $(TARGETDIR)/, $(TARGETS))

all: $(FULL_TARGETS)

clean:
	@$(RM) -rf $(TARGETDIR)/*.so

$(TARGETDIR)/%.so: %.cc %.h
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@
