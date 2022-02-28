CXX		:= clang
CXXFLAGS:= -Weverything -Wno-padded -Wno-unused-macros -Wno-switch-enum -Iinc/
SOURCES	:= $(wildcard src/*.c)
OBJECTS	:= $(patsubst src/%,obj/%,$(patsubst %.c,%.o,$(SOURCES)))
DEPENDS	:= $(patsubst src/%,obj/%,$(patsubst %.c,%.d,$(SOURCES)))
TARGET	:= attis

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(OBJECTS) $(DEPENDS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@ -lm

-include $(DEPENDS)

obj/%.o: src/%.c Makefile
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
