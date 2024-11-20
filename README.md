# Awn
Awn is a personal C++ library designed to power low-latency multi-threaded real-time applications.

# Awn Features
* An asynchronous file resource manager for real-time file streaming
* A parameter system to autogenerate and manage parameter classes for binary data using the byaml file format
* A platform agnositic threading api implementing synchronization primitives using modern techniques (atomics, WaitOnAddress)
* A CPU and GPU heap tree memory manager for real-time allocations and memory monitoring
* A graphics api based around supporting file formats reversed from nn::gfx
* Basic mouse and keyboard support

# Vp
Vp is a personal C++ library designed to portably carry common utilities and file format definitions.

# Vp Features
* Reverse engineered file headers, accessors, and serializers for various NintendoWare and Nintendo EPD formats (names may not be official)
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
* Zstd streaming decompression and Yaz0 decompressor for Nintendo szs files
* Hash algorithms, crc32b, murmur3, and murmur64
* Avx2 SIMD linear algebra algorithms for fast 3d transformations and projections
* Utilities
  * Scope Guard
  * Result-based Structured Error Handling
  * Custom Runtime Type Info
  * Utf8, Utf16, and Utf32 string encoding conversion
  * File Path Parsing
  * Enum Reflection
* Data Structures
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

# Wrapping up development (Finalizing)
* A multi-threaded main loop based on dependency graphs
* Acpcm to Pcm16 decoding and Pcm16 to Pcm24 and Pcm Float conversion
* Audio device management, input, and output

# In Development (Prototyping)
* An audio renderer for mixing, applying effects, and outputing pcm16, adpcm, and opus encoded audio
* An Actor and Scene system
* An AI behavior tree system for scripting actor and scene behavior
* A model scene system for managing the rendering of 3d models
* A UI and Font system implementing the reverse engineered bflyt format
* Wiimote, Npad (Nintendo Switch), PS4, and XInput controller support

# System Requirements
* The user's Vulkan implementation must conform to all the limits and features enforced by awn::gfx::Context which include but are not limited to:
  * Device Addresses
  * Descriptor Buffers
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

This repo is currently not all of the project but irregular snapshots of passable parts.

If there are problems of any kind please file an issue or contact me and I'll do my best to resolve the issue.
