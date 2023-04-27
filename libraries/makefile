
# Defaults
ARCHITECTURE ?= x86
PLATFORM     ?= win32
GRAPHICS_API ?= vulkan
BINARY_TYPE  ?= debug

# User component lists
GENERIC_SUB_LIST := vp

win32_SUB_LIST   := awn

define MAKE_GENERIC
$(MAKE) ARCHITECTURE=$(ARCHITECTURE) PLATFORM=$(PLATFORM) GRAPHICS_API=$(GRAPHICS_API) BINARY_TYPE=$(BINARY_TYPE) -C lib_$(1)

endef
define MAKE_PLATFORM
$(MAKE) ARCHITECTURE=$(ARCHITECTURE) PLATFORM=$(PLATFORM) GRAPHICS_API=$(GRAPHICS_API) BINARY_TYPE=$(BINARY_TYPE) -C lib_$(1)_$(2)

endef

sub_make:
	$(foreach program,$(GENERIC_SUB_LIST),$(call MAKE_GENERIC,$(program)))
	$(foreach program,$($(PLATFORM)_SUB_LIST),$(call MAKE_PLATFORM,$(program),$(PLATFORM)))

clean:
	