# Graphics api flags 

THIRD_PARTY_DIRS :=

export GFXAPI_INCLUDES     := $(foreach dir,$(THIRD_PARTY_DIRS),-I$(dir)/include)
export GFXAPI_LIB_INCLUDES := $(foreach dir,$(THIRD_PARTY_DIRS),-L$(dir)/lib)
