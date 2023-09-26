
# Pull in architecture
ifeq ($(ARCHITECTURE), x86)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_x86.mk
else
ifeq ($(ARCHITECTURE), aarch64)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_aarch64.mk
else
ifeq ($(ARCHITECTURE), armv7)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_armv7.mk
else
ifeq ($(ARCHITECTURE), geckoppc)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_geckoppc.mk
else
ifeq ($(ARCHITECTURE), armv6k)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_armv6k.mk
else
ifeq ($(ARCHITECTURE), armv5te)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_armv5te.mk
else
ifeq ($(ARCHITECTURE), armv4t)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_armv4t.mk
else
ifeq ($(ARCHITECTURE), mips64)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_mips64.mk
else
$(error Invalid ARCHITECTURE (check "config/arch" for valid list))
endif
endif
endif
endif
endif
endif
endif
endif

# Pull in platform
ifeq ($(PLATFORM), win32)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_win32.mk
else
ifeq ($(PLATFORM), winnt)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_winnt.mk
else
ifeq ($(PLATFORM), nx)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_nx.mk
else
ifeq ($(PLATFORM), cafe)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_cafe.mk
else
ifeq ($(PLATFORM), ktr)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_ktr.mk
else
ifeq ($(PLATFORM), ctr)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_ctr.mk
else
ifeq ($(PLATFORM), twl)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_twl.mk
else
ifeq ($(PLATFORM), rvl)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_rvl.mk
else
ifeq ($(PLATFORM), dol)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_dol.mk
else
ifeq ($(PLATFORM), agb)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_agb.mk
else
ifeq ($(PLATFORM), nus)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_nus.mk
else
$(error Invalid PLATFORM (check "config/platform" for valid list))
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif

# Pull in graphics api
ifeq ($(GRAPHICS_API), vk)
include $(dir $(lastword $(MAKEFILE_LIST)))../graphics_api/gfxapi_vk.mk
else
ifeq ($(GRAPHICS_API), nvn)
include $(dir $(lastword $(MAKEFILE_LIST)))../graphics_api/gfxapi_nvn.mk
else
ifeq ($(GRAPHICS_API), awngfx)
# ...
else
$(error Invalid GRAPHICS_API (check "config/gfxapi" for valid list))
endif
endif
endif

# Pull in binary type
ifeq ($(BINARY_TYPE), release)
include $(dir $(lastword $(MAKEFILE_LIST)))../binary_type/bintype_release.mk
else
ifeq ($(BINARY_TYPE), debug)
include $(dir $(lastword $(MAKEFILE_LIST)))../binary_type/bintype_debug.mk
else
ifeq ($(BINARY_TYPE), develop)
include $(dir $(lastword $(MAKEFILE_LIST)))../binary_type/bintype_develop.mk
else
$(error Invalid BINARY_TYPE (must be "release", "develop", or "debug"))
endif
endif
endif

export ALL_COMPILER_FLAGS := $(ARCH_CXX_FLAGS) $(PLATFORM_C_FLAGS) $(PLATFORM_CXX_FLAGS) $(PLATFORM_INCLUDES) $(PLATFORM_LIB_INCLUDES) $(GFXAPI_INCLUDES) $(GFXAPI_LIB_INCLUDES) $(BINTYPE_CXX_FLAGS) $(PROJECT_C_FLAGS) $(PROJECT_CXX_FLAGS) $(PROJECT_INCLUDES) $(PROJECT_LIB_INCLUDES)

export ALL_LIBS := $(PLATFORM_LIBS) $(PROJECT_LIBS)

ifneq ($(strip $(VULKAN_SDK)),)
GLSLC ?= $(VULKAN_SDK)/bin/glslc.exe
endif

# Compile recipes

# Vulkan shader build rules
ifeq ($(GRAPHICS_API), vk)
%.vert.spv : %.vert.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.vert.d -DDD_VERTEX_SHADER                  -mfmt=bin --target-env=vulkan1.3 -fshader-stage=vert        -I$(CURDIR)/../shader/include -Werror -Os -c $< -o $(CURDIR)/../build/resource/shader/$*.vert.spv

%.frag.spv : %.frag.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.frag.d -DDD_FRAGMENT_SHADER                -mfmt=bin --target-env=vulkan1.3 -fshader-stage=frag        -I$(CURDIR)/../shader/include -Werror -Os -c $< -o $(CURDIR)/../build/resource/shader/$*.frag.spv

%.tsct.spv : %.frag.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.tsct.d -DDD_TESSELLATION_CONTROL_SHADER    -mfmt=bin --target-env=vulkan1.3 -fshader-stage=tesscontrol -I$(CURDIR)/../shader/include -werror -O -c $< -o $(CURDIR)/../build/resource/shader/$*.tsct.spv

%.tsev.spv : %.frag.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.tsev.d -DDD_TESSELLATION_EVALUATION_SHADER -mfmt=bin --target-env=vulkan1.3 -fshader-stage=tesseval    -I$(CURDIR)/../shader/include -werror -O -c $< -o $(CURDIR)/../build/resource/shader/$*.tsev.spv

%.geom.spv : %.frag.sh
	@echo $(notdir $<)	
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.geom.d -DDD_GEOMETRY_SHADER                -mfmt=bin --target-env=vulkan1.3 -fshader-stage=geom        -I$(CURDIR)/../shader/include -werror -O -c $< -o $(CURDIR)/../build/resource/shader/$*.geom.spv

%.comp.spv : %.frag.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.comp.d -DDD_COMPUTE_SHADER                 -mfmt=bin --target-env=vulkan1.3 -fshader-stage=comp        -I$(CURDIR)/../shader/include -werror -O -c $< -o $(CURDIR)/../build/resource/shader/$*.comp.spv
endif

# Windows exe build rules
ifeq ($(PLATFORM), win32)
%.exe:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(EXE_FLAGS) -o $@ $^ $(ALL_LIBS)
endif
ifeq ($(PLATFORM), winnt)
%.exe:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(EXE_FLAGS) -o $@ $^ $(ALL_LIBS)
endif

# Nintendo Switch build rules
ifeq ($(PLATFORM), nx)
ifeq ($(ARCHITECTURE), aarch64)
%.nro:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(NRO_FLAGS) -o $@ $^ $(ALL_LIBS)
endif
endif

# Nintendo Wii U build rules
ifeq ($(PLATFORM), cafe)
%.rpx:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(NRO_FLAGS) -o $@ $^ $(ALL_LIBS)
endif

# Nintendo 3ds build rules
ifeq ($(PLATFORM), ctr)
%.geom.sh:
%.vert.sh:

%.cia:

%.3dsx:

%.elf:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(NRO_FLAGS) -o $@ $^ $(ALL_LIBS)
	
endif

# Nintendo DS build rules
ifeq ($(PLATFORM), ntr)
%.nds:
	

%.elf:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(NRO_FLAGS) -o $@ $^ $(ALL_LIBS)
endif

# Nintendo Wii and GameCube build rules
ifeq ($(PLATFORM), dol)
%.iso:

%.dol:

%.elf:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(NRO_FLAGS) -o $@ $^ $(ALL_LIBS)
endif

# Nintendo GameBoy Advance build rules
ifeq ($(PLATFORM), agb)
%.gba:

%.elf:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(NRO_FLAGS) -o $@ $^ $(ALL_LIBS)
endif

# Nintendo 64 build rules
ifeq ($(PLATFORM), nus)
%.z64:
	@echo Creating rom $@
	$(N64_TOOL) $(N64TOOLFLAGS) -o $@ $^
	$(CHECKSUM64) $@

%.bin:
	@echo Copying obj from elf $@
	$(COMPILER_PREFIX)$(OBJ_COPY) $< $@ -O binary

%.elf:
	@echo Compiling $@
	$(COMPILER_PREFIX)$(CXX) $(ALL_COMPILER_FLAGS) $(Z64_FLAGS) -o $@ $^ $(ALL_LIBS)
endif

# Common build rules
%.o : %.cpp
	@echo $(notdir $<)
	@$(COMPILER_PREFIX)$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(ALL_COMPILER_FLAGS) -c $< -o $@ $(ALL_LIBS)

%.a :
	@rm -f $@
	$(COMPILER_PREFIX)$(AR) -rc $@ $^

%.hpp.gch: %.hpp
	@echo Precompiling $(notdir $<)
	$(COMPILER_PREFIX)$(CXX) -w -x c++-header -MMD -MP -MQ$@ -MF $(DEPSDIR)/$(notdir $*).d $(ALL_COMPILER_FLAGS) -c $< -o $@ $(ALL_LIBS)

