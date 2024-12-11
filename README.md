# Awn
Awn is a personal C++ library designed to power low-latency multi-threaded real-time applications.

# Awn Features
* awn::mem; Provides virtual and heap-based CPU and GPU memory management
* awn::sys; Provides thread and system environment management (managing Win32 or Horizon)
* awn::async; Provides thread and system environment management (managing Win32 or Horizon)
* awn::res; Provides synchronous and asynchronous file resource management
* awn::gfx; Provides a high-level graphics api (managing Vulkan or NVN), designed to interoperate with NintendoWare gfx formats
* awn::render; Provides display and layer management (managing Win32 windows, DXGI, DirectComposition, and DirectX12, or nn::vi)
* awn::para; Provides serializable parameter class autogen, databases, blackboards, pausing, variable framerates, etc
* awn::hid; Provides host input device management for keyboards and mice
* awn::frm; Provides a bootstrap for libraries

# Vp
Vp is a personal C++ library designed to portably carry common utilities and file format definitions.

# Vp Features
* vp::res, Reverse engineered file headers, accessors, and serializers for various NintendoWare and Nintendo EPD formats (names may not be official)
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
* vp::resbui, serializers for select file formats
  * Bnsh (Binary Nintendo Shader)
  * Byaml (Binary Yet Another Markup Language)
  * Rsizetable (Resource Size Table)
  * Sarc (sead Archive?)
* vp::util, Data structures, algorithms, traits
   * Hash algorithms, crc32b, murmur3, and murmur64
   * x86_64 avx2 SIMD linear algebra
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

This repo is currently not all of the project but irregular snapshots of passable parts.

If there are problems of any kind please file an issue or contact me and I'll do my best to resolve the issue.
