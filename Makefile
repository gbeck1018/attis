CXX		:= clang
CXXFLAGS:= -Weverything -g \
-Wno-padded -Wno-unused-macros -Wno-switch-enum -Wno-language-extension-token \
-Wno-cast-align \
-Iinc \

SRCDIR := src/
OBJDIR := obj/

SOURCES := $(shell find $(SRCDIR) -type f -name '*.c')
OBJECTS	:= $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.c,%.o,$(SOURCES)))
DEPENDS	:= $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.c,%.d,$(SOURCES)))
TARGET	:= attis

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) -r $(OBJDIR) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@ -lm

-include $(DEPENDS)

$(OBJDIR)%.o: $(SRCDIR)%.c Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
