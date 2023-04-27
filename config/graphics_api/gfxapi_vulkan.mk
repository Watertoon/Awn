# Graphics api flags 
ifeq ($(strip $(VULKAN_SDK)),)
$(error "VULKAN_SDK is not set in your environment.")
endif

THIRD_PARTY_DIRS := $(VULKAN_SDK)

export GFXAPI_INCLUDES     := $(foreach dir,$(THIRD_PARTY_DIRS),-I$(dir)/include)
export GFXAPI_LIB_INCLUDES := $(foreach dir,$(THIRD_PARTY_DIRS),-L$(dir)/lib)
