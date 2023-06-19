# Platform config

THIRD_PARTY_DIRS :=

export PLATFORM_C_FLAGS      := 
export PLATFORM_CXX_FLAGS    := 
export PLATFORM_LIB_INCLUDES := $(foreach dir,$(THIRD_PARTY_DIRS),-I$(dir)/include)
export PLATFORM_INCLUDES     := $(foreach dir,$(THIRD_PARTY_DIRS),-L$(dir)/lib)
export PLATFORM_LIBS         := -static-libgcc -static-libstdc++ -lShlwapi -lkernel32 -lSynchronization
