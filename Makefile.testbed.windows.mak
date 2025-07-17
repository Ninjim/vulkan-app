# This doesn't check for changed headers?

# For windows so that they are all with the appropriate backslashes
DIR := $(subst /,\,${CURDIR}) 
BUILD_DIR := bin
OBJ_DIR := obj
DEP_DIR := dep

ASSEMBLY := testbed
EXTENSION := .exe
COMPILER_FLAGS := -g -fdeclspec -Werror=vla -Wno-missing-braces -MP -MD -MF #-fPIC for linux
INCLUDE_FLAGS := -Iengine\src -Itestbed\src
# can i not leave out engine.lib?
LINKER_FLAGS := -g -lengine.lib -L$(BUILD_DIR) -L$(OBJ_DIR)\engine # idk why this is included I'll see if I can remove it?
DEFINES := -D_DEBUG -DKIMPORT

# Recursive function for Windows
# The first wildcard just outputs all .c files in that directory, the second searches for subdirectories, if it is a .c file the d/ fails
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/$2))

# change to backslash?
# gets all the c files
SRC_FILES := $(call rwildcard,$(ASSEMBLY)/,*.c)

# gets all directories, dir is for getting the contents of a directory, /S is for looking in subdirectories, /AD is for spewing out only directories, /B means no volume e.g. stuff like how
# much memory inside a folder yadda yadda so we get only one line full paths
# subst removes the full path prefix and gives only relative paths
DIRECTORIES := \$(ASSEMBLY)\src $(subst $(DIR),,$(shell dir $(ASSEMBLY)\src /S /AD /B | findstr /i src))
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o) # gets path and names for obj files
DEP_FILES := $(SRC_FILES:%=$(DEP_DIR)/$(basename %).d)


all: scaffold compile link

.PHONY: scaffold

scaffold: # creating the directories, enableextensions gives more shell calls like "call" and "mkdir", 2>NUL means to not give any errors? || cd . is basically saying to pipe everything to cd . which is a useless command, so ignore the info it says?, enabledelayedexpansion doesn't actually do anything I think
	@echo Scaffolding folder structure...
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(OBJ_DIR), $(DIRECTORIES)) 2>NUL || cd .
		-@setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(DEP_DIR), $(DIRECTORIES)) 2>NUL || cd .
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES)
	@echo Linking $(ASSEMBLY)...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile:
	@echo Compiling...

.PHONY: clean
clean: # /s makes it do subdirectories /q quiets messages like "Are you sure you want to delete this?"
	if exist $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) del $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION)
	rmdir /s /q $(OBJ_DIR)\$(ASSEMBLY) 

# change backslash?
$(OBJ_DIR)/%.c.o: %.c 
	@echo $<...
	@clang $< $(COMPILER_FLAGS) $(DEP_DIR)\$(basename $<).d -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

-include $(DEP_FILES)