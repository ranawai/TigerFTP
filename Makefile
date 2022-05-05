TARGET_EXEC ?= TigerC

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src/library
CLI_DIRS ?= ./src/client
INC_OTHER ?= ./include

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c)
CLI_SRC := ./src/client/TigerC.c

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o) $(CLI_SRC:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(INC_OTHER) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

INCL_DIRS := $(shell find $(SRC_DIRS) -type d) $(shell find $(CLI_DIRS) -type d)
INCL_FLAGS := $(addprefix -I,$(INCL_DIRS))


CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g --std=c++14
CCFLAGS ?= $(INC_FLAGS) -MMD -MP -g

# NOTE: Add the  -lpthread -lrt  library flags after the $(LDFLAGS) 
# if pthreads are used in Linux

#LDFLAGS ?= $(LDFLAGS) -lpthread -lrt

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@echo "source found: $(SRCS) $(CLI_SRC)"
	@echo "objects found: $(OBJS)"
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Do nothing for the .re file - this is a manual process
$(BUILD_DIR)/%.re.o: %.re
	

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CCFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
