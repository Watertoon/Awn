# Platform flags

export COMPILER_PREFIX := mips64-elf-
export ARCH_CXX_FLAGS  := -mtune=vr4300 -march=vr4300

export OBJ_COPY        := objcopy
export N64_TOOL        := n64tool
export CHECKSUM64      := chksum64
