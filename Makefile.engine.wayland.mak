
BUILD_DIR := bin
OBJ_DIR := obj
DEP_DIR := dep

ASSEMBLY := engine
EXTENSION := .so
COMPILER_FLAGS := -g -MD -Werror=vla -fdeclspec -fPIC -MP -MF
INCLUDE_FLAGS := -Iengine/src -I$(VULKAN_SDK)/include
LINKER_FLAGS := -g -shared -lwayland-client -lrt -lglfw -L$(VULKAN_SDK)/lib -lvulkan -Wl,-rpath,.
DEFINES := -D_DEBUG -DKEXPORT -DKWAYLAND

# Make does not offer a recursive wildcard function, so here's one:
#rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(shell find $(ASSEMBLY) -name *.c)		# .c files
DIRECTORIES := $(shell find $(ASSEMBLY) -type d)		# directories with .h files
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o)		# compiled .o objects
DEP_FILES := $(SRC_FILES:%=$(DEP_DIR)/$(basename %).d)

all: scaffold compile link

.PHONY: scaffold
scaffold: # create build directory
	@echo Scaffolding folder structure...
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(addprefix $(OBJ_DIR)/,$(DIRECTORIES))
	@mkdir -p $(addprefix $(DEP_DIR)/, $(DIRECTORIES))
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES) # link
	@echo Linking $(ASSEMBLY)...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)/lib$(ASSEMBLY)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile: #compile .c files
	@echo Compiling...
-include $(OBJ_FILES:.o=.d)
.PHONY: clean
clean: # clean build directory
	rm -rf $(BUILD_DIR)/
	rm -rf $(OBJ_DIR)/$(ASSEMBLY)

$(OBJ_DIR)/%.c.o: %.c # compile .c to .o object
	@echo   $<...
	@clang $< $(COMPILER_FLAGS) $(DEP_DIR)/$(basename $<).d -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

-include $(DEP_FILES)