########################################################################
####################### Makefile Template ##############################
########################################################################

# Tool settings
CC = gcc
AR = ar
ARFLAGS = rcs
CXXFLAGS = -std=c11 -Wall -g -I$(INCDIR)
LDFLAGS = 

# Makefile settings - Can be customized
APPNAME = program
LIBNAME = lib$(APPNAME)
EXT = c
SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = src/include
LIBDIR = lib

########################################################################
############## Instructions On How To Use This Make File ###############
########################################################################

# ## Recommended File Structure:
# .
# ├── bin/             (Final binaries and libraries are placed here)
# ├── obj/             (Object files .o and dependency files .d)
# ├── lib/             (External .a library files go here)
# ├── src/
# │   ├── include/     (Your project's header files .h)
# │   └── ...          (Your source code files .c)
# └── Makefile

# ## Available Commands:
# make all          (or just 'make') Builds the executable.
# make library      Builds the project as a static library (.a file).
# make clean        Removes all generated files (obj, bin).

# ## Linking External Libraries:
# This Makefile automatically links any .a library files found in the
# directory specified by LIBDIR (default: 'lib/').
# Simply place your .a files there and run 'make'.
#
# To link system libraries (e.g., the math library), add them
# manually to LDFLAGS. Example:
# LDFLAGS += -lm

########################################################################
############## Do not change anything from here downwards! #############
########################################################################

# Automatic library linking logic
LIBS_PATH := $(shell find $(LIBDIR) -maxdepth 1 -name '*.a' 2>/dev/null)
ifneq ($(LIBS_PATH),)
  LIB_NAMES := $(notdir $(LIBS_PATH))
  LIB_NAMES := $(patsubst %.a,%,$(LIB_NAMES))
  LIB_NAMES := $(patsubst lib%,%,$(LIB_NAMES))
  L_FLAGS   := $(addprefix -l,$(LIB_NAMES))
  LDFLAGS   += -L$(LIBDIR) $(L_FLAGS)
endif

# Recursively find all source files in SRCDIR
SRC = $(shell find $(SRCDIR) -name '*.$(EXT)')

# Generate object and dependency file lists
OBJ = $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEP = $(OBJ:.o=.d)

# OS-specific variables
RM = rm
DEL = del
EXE = .exe

####################### Targets beginning here #########################

.PHONY: all library clean cleanw

all: $(BINDIR)/$(APPNAME)

library: $(BINDIR)/$(LIBNAME).a

# Main build targets
$(BINDIR)/$(APPNAME): $(OBJ)
	@mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BINDIR)/$(LIBNAME).a: $(OBJ)
	@mkdir -p $(BINDIR)
	$(AR) $(ARFLAGS) $@ $^

# Pattern rules for objects and dependencies
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< > $@

# Include dependency files if they exist
-include $(DEP)

# Cleaning rules
clean:
	$(RM) -rf $(OBJDIR) $(BINDIR)

cleanw:
	-if exist $(OBJDIR) rd /s /q $(OBJDIR)
	-if exist $(BINDIR) rd /s /q $(BINDIR)