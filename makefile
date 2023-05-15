.SUFFIXES:

.PHONY: all clean

# This is the toplevel makefile for the following sub directories

export LIBRARY_DIR_LIST     := libraries
export PROGRAM_DIR_LIST     := 
export TOOL_DIR_LIST        :=
export THIRD_PARTY_DIR_LIST := 
export UNIT_TEST_DIR_LIST   := 

ALL_PROGRAMS_LIST := $(THIRD_PARTY_LIST) $(LIBRARY_DIR_LIST) $(TOOL_LIST) $(PROGRAM_LIST) $(UNIT_TEST_LIST)


# Default build environment variables to be overriden here or from the command line
arch         ?= x86
platform     ?= win32
graphics_api ?= vk
binary_type  ?= develop

#Valid Architectures: x86, Aarch64, Arm, PowerPC, Mips
#Valid platforms:     win32, winnt, nx, ctr, cafe, rvl, dol, ntr, nus, agb
#Valid graphics apis: vulkan, nvn, awn
#Valid binary types:  release develop debug

define VP_SUBMAKE
+$(MAKE) sub_make ARCHITECTURE=$(arch) PLATFORM=$(platform) GRAPHICS_API=$(graphics_api) BINARY_TYPE=$(binary_type) -C $(1)

endef

define MAKE_CLEAN
$(MAKE) clean -C $(1)

endef

all:
	$(foreach program,$(ALL_PROGRAMS_LIST),$(call VP_SUBMAKE,$(program)))

clean:
	$(foreach program,$(ALL_PROGRAMS_LIST),$(call MAKE_CLEAN,$(program)))

