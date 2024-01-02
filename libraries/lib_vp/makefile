# By W. Michael

# Set default if unset
ARCHITECTURE ?= x86
PLATFORM     ?= win32
GRAPHICS_API ?= vk
BINARY_TYPE  ?= develop

# Pull in common config
include $(BUILD_RULE_DIR)../../config/common.mk

# Common directory iterators
DIRECTORY_WILDCARD  =$(foreach d,$(wildcard $(1:=/*)),$(if $(wildcard $d/.),$(call DIRECTORY_WILDCARD,$d) $d,))
GET_ALL_SOURCE_DIRS =$1 $(foreach d,$(wildcard $1/*),$(if $(wildcard $d/.),$(call DIRECTORY_WILDCARD,$d) $d,))
FIND_SOURCE_FILES   =$(foreach dir,$1,$(notdir $(wildcard $(dir)/*.$2)))
FIND_TARGET_FILES   =$(foreach dir,$1,$(notdir $(wildcard $(dir)/*.*.$2)))

# Get source and shader source directories
SOURCE_DIRS=$(call GET_ALL_SOURCE_DIRS,source)
SHADER_SOURCE_DIRS=$(call GET_ALL_SOURCE_DIRS,shader/source)

ifneq ($(BUILD_DIR),$(notdir $(CURDIR)))

# User program options (edit these)
CXX_DEFINES  := -DVP_DEBUG
CXX_FLAGS    := -std=gnu++20 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -fno-rtti -fno-exceptions $(CXX_DEFINES)
CXX_WARNS    := -Wall -Wno-format-truncation -Wno-format-zero-length -Wno-stringop-truncation -Wno-invalid-offsetof -Wno-format-truncation -Wno-format-zero-length -Wno-stringop-truncation -Wextra -Werror -Wno-missing-field-initializers
LIBRARY_DIRS := 

export PROJECT_C_FLAGS      := 
export PROJECT_CXX_FLAGS    := $(RELEASE_FLAGS) $(CXX_FLAGS) $(CXX_WARNS) -DVP_TARGET_PLATFORM_$(PLATFORM) -DVP_TARGET_ARCHITECTURE_$(ARCHITECTURE) -DVP_TARGET_GRAPHICS_API_$(GRAPHICS_API)
export PROJECT_INCLUDE_DIRS := include third_party/include
export PROJECT_LIB_INCLUDES := $(foreach dir,$(LIBRARY_DIRS),-I$(dir)/lib)
export PROJECT_INCLUDES     := $(foreach dir,$(PROJECT_INCLUDE_DIRS),-I$(CURDIR)/$(dir)) \
					           $(foreach dir,$(LIBRARY_DIRS),-I$(dir)/include) \
					           -I.
export PROJECT_LIBS			:= -static -lzstd

# Filter source files to exclude the unselected targets in the format (file-name).(target platform, arch, binary type, or gfxapi).cpp
UNFILTERED_CPP_FILES  := $(call FIND_SOURCE_FILES,$(SOURCE_DIRS),cpp)
TARGET_CPP_FILES      := $(call FIND_TARGET_FILES,$(SOURCE_DIRS),cpp)
FILTERED_CPP_FILES    := $(filter-out $(TARGET_CPP_FILES),$(UNFILTERED_CPP_FILES))
FILTERED_CPP_FILES    += $(filter %.$(PLATFORM).cpp,$(UNFILTERED_CPP_FILES))
FILTERED_CPP_FILES    += $(filter %.$(ARCHITECTURE).cpp,$(UNFILTERED_CPP_FILES))
FILTERED_CPP_FILES    += $(filter %.$(GRAPHICS_API).cpp,$(UNFILTERED_CPP_FILES))
FILTERED_CPP_FILES    += $(filter %.$(BINARY_TYPE).cpp,$(UNFILTERED_CPP_FILES))

# Export source files
export CPP_FILES := $(FILTERED_CPP_FILES)
export O_FILES   := $(CPP_FILES:.cpp=.o)

# Filter shader source files
UNFILTERED_SH_FILES := $(call FIND_SOURCE_FILES,$(SHADER_SOURCE_DIRS),sh)

# Export shader files
export VERTEX_SH_FILES                   := $(filter %.vert.sh,$(UNFILTERED_SH_FILES))
export TESSELLATION_CONTROL_SH_FILES     := $(filter %.tsct.sh,$(UNFILTERED_SH_FILES))
export TESSELLATION_EVALUATION_SH_FILES  := $(filter %.tsev.sh,$(UNFILTERED_SH_FILES))
export GEOMETRY_SH_FILES                 := $(filter %.geom.sh,$(UNFILTERED_SH_FILES))
export FRAGMENT_SH_FILES                 := $(filter %.frag.sh,$(UNFILTERED_SH_FILES))
export COMPUTE_SH_FILES                  := $(filter %.comp.sh,$(UNFILTERED_SH_FILES))

export VERTEX_SPV_FILES                  := $(VERTEX_SH_FILES:.sh=.spv)
export TESSELLATION_CONTROL_SPV_FILES    := $(TESSELLATION_CONTROL_SH_FILES:.sh=.spv)
export TESSELLATION_EVALUATION_SPV_FILES := $(TESSELLATION_EVALUATION_SH_FILES:.sh=.spv)
export GEOMETRY_SPV_FILES                := $(GEOMETRY_SH_FILES:.sh=.spv)
export FRAGMENT_SPV_FILES                := $(FRAGMENT_SH_FILES:.sh=.spv)
export COMPUTE_SPV_FILES                 := $(COMPUTE_SH_FILES:.sh=.spv)


# Export precompiled headers
export PRECOMPILED_HEADERS  := $(CURDIR)/include/vp.hpp
export GCH_FILES			:= $(PRECOMPILED_HEADERS:.hpp=.hpp.gch)

# Export prequisite paths
export VPATH := $(foreach dir,$(SOURCE_DIRS),$(CURDIR)/$(dir))\
                $(foreach dir,$(SHADER_SOURCE_DIRS),$(CURDIR)/$(dir))\
                $(CURDIR)/include

# Formatted for recipes
export BUILD_LIBS := vp
export BUILD_EXES :=

export BUILD_RULE_DIR := $(CURDIR)/

.PHONY: all clean release_deps shader_deps build build/resources build/resources/shaders lib lib/$(BUILD_LIBS).a build/$(BUILD_EXES).exe

# Build settings to set required variables (call these for non-defaults)

sub_make:
	$(MAKE) all PLATFORM=$(PLATFORM) ARCHITECTURE=$(ARCHITECTURE) GRAPHICS_API=$(GRAPHICS_API) BINARY_TYPE=$(BINARY_TYPE)

# Output variation(s) to be used (edit this)

all: lib/$(BUILD_LIBS).a

# Clean
clean:
	@echo cleaning ...
	@rm -fr build release_deps shader_deps lib $(GCH_FILES)

# Output folders

list:
	@echo $(UNFILTERED_CPP_FILES)
	@echo $(FILTERED_CPP_FILES)
	@echo $(CPP_FILES)
	@echo $(O_FILES)
	@echo $(TARGET_CPP_FILES)
	@echo $(SOURCE_DIRS)
	@echo $(SHADER_SOURCE_DIRS)

release_deps:
	@[ -d $@ ] || mkdir -p $@

shader_deps:
	@[ -d $@ ] || mkdir -p $@

build:
	@[ -d $@ ] || mkdir -p $@

build/resource:
	@[ -d $@ ] || mkdir -p $@

build/resource/shader:
	@[ -d $@ ] || mkdir -p $@

lib:
	@[ -d $@ ] || mkdir -p $@

# Binary output

lib/$(BUILD_LIBS).a: lib release_deps
	@$(MAKE) BUILD_DIR=release_deps OUTPUT=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DNDEBUG=0" \
	DEPSDIR=$(CURDIR)/release_deps \
	-C release_deps \
	-f $(CURDIR)/makefile

build/$(BUILD_EXES).exe: release_deps build $(SOURCE_DIRS)
	@$(MAKE) BUILD_DIR=release_deps OUTPUT=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DNDEBUG=0" \
	DEPSDIR=$(CURDIR)/release_deps \
	-C release_deps \
	-f $(CURDIR)/makefile

shader: shader_deps build/resource/shader $(SHADER_SOURCE_DIRS)
	@$(MAKE) BUILD_DIR=shader_deps \
	DEPSDIR=$(CURDIR)/shader_deps \
	-C shader_deps \
	-f $(CURDIR)/makefile
	@echo $(OUTPUT)

else

# Check for shader build
ifneq ($(findstring $(BUILD_DIR),shader_deps),)

DEPENDS := $(SPV_FILES:.spv=.d)

all: $(COMPUTE_SPV_FILES) $(FRAGMENT_SPV_FILES) $(GEOMETRY_SPV_FILES) $(TESSELLATION_EVALUATION_SPV_FILES) $(TESSELLATION_CONTROL_SPV_FILES) $(VERTEX_SPV_FILES)

-include $(DEPENDS)

else

DEPENDS := $(O_FILES:.o=.d) $(foreach hdr,$(GCH_FILES:.hpp.gch=.d),$(notdir $(hdr)))

$(OUTPUT) : $(O_FILES)

$(O_FILES) : $(GCH_FILES)

-include $(DEPENDS)

endif
endif
