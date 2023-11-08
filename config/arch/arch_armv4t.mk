# Platform flags

export COMPILER_PREFIX := arm-none-eabi-
export ARCH_CXX_FLAGS  := -mtune=arm7tdmi -march=armv4t -mfloat-abi=soft -mlittle-endian -mthumb -mthumb-interwork -DVP_32_BIT
