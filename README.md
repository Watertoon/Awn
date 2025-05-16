# Awn
Awn is a personal C++ library designed to power low-latency multi-threaded real-time applications.

# Awn Features
* awn::mem; Memory management library, provides heap data structures and virtual memory management
* awn::sys; Underlying operating environment library, manages threading and other common OS operations across supported platforms (Win32 or NX)
* awn::async; Multithreaded work scheduling library, provides asynchronous task management and dependency-based job graphs
* awn::res; File resource library; provides an asynchronous resource loader for allocating, loading, decompressing, and initializing file data
* awn::gfx; High-level graphics api, abstracts platform graphics api (Vulkan or NVN), interoperates with reverse-engineered NintendoWare gfx formats
* awn::grr; Graphics rendering library, Provides display management (Win32 Composition Swapchain or nn::vi)
* awn::para; Parameterization library, class autogen for general byaml parsing, blackboards, pause management, variable framerate management, etc
* awn::hid; Host input device management, preliminary supports keyboard and mice
* awn::frm; Provides a bootstrap for libraries

# Vp
Vp is a personal C++ library designed to portably carry common utilities and file format definitions.

# Vp Features
* vp::codec; compression (zstd), encryption (nisasyst, aes128cbc)
* vp::imem; Heap interface
* vp::res; Resource library, reverse engineered file headers, accessors, and serializers for various NintendoWare and Nintendo EPD formats (names may not be official)
  * Ainb (AI Node Binary)
  * Bea (Bezel Engine Archive)
  * Bffnt (Binary Cafe Font)
  * Bfres (Binary Cafe Resource)
  * Bfsha (Binary Cafe Shader Archive)
  * Bnsh (Binary Nintendo Shader)
  * Bntx (Binary Nintendo Texture Archive)
  * Byaml (Binary Yet Another Markup Language)
  * Bwav (Binary Wave)
  * Common nn::util and NintendoWare gfx structures and enums
  * Exb (Expression Binary)
  * Rsizetable (Resource Size Table)
  * Sarc (sead Archive?)
* vp::resbui, Resource serialization library, serializers for:
  * Bnsh (Binary shader format)
  * Bntx (Binary texture archive format)
  * Byaml (Binary YAML)
  * Rsizetable (Resource size table)
  * Sarc (Archive format)
* vp::util, General utility library:
  * Hash algorithms, crc32b, murmur3, and murmur64
  * x86_64 avx2 SIMD math
  * Scope Guard
  * Result-based Structured Error Handling
  * Custom Runtime Type Info
  * Utf8, Utf16, and Utf32 string encoding conversion
  * File Path Parsing
  * Enum Reflection
  * Red Black Trees
  * Ring Buffers
  * Handle Tables 
  * Object Allocators
  * Index Allocators
  * Intrusive Lists
  * Priority Queues
  * Hash Maps
  * Bit arrays
  * Pointer Arrays
  * Frame Arrays
  * Heap Arrays
  * Fixed Strings
  * IFunction

# In Development
* An immediate mode style draw api
* 3d model and animation support

# System Requirements
* The user's Vulkan implementation must conform to all the limits and features enforced by awn::gfx::Context which include but are not limited to:
  * Descriptor Buffers
  * Mutable Descriptors
  * Shader Objects
  * Resizable-Bar
* Requires an x86_64 CPU with AVX2 and PCLMUL

# Footnote
Built for Windows 10 with GCC 14.1 and gnu make from MSYS2. 
Build instructions:
1. Install MSYS2 
2. From MSYS2 install the standard GCC package
3. Run the newly installed Make program from the command line in the root project directory
4. Link the relevant libraries in other programs

I'm informed by reverse engineering, text books, free online resources, word on the street, and api documentation. I believe to be conformant with respect to my references.

This repo is currently not all of the project but irregular snapshots.

Feel free to contact me or file an issue.
