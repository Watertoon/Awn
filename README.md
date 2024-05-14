# Awn
Awn is a personal C++ library designed to power low-latency multi-threaded real-time applications.

# Vp
Vp is an included personal C++ library designed to carry common utilities and file format definitions in a more portable form than Awn.

# Vp Features
* Linear Algebra math library
* Reverse engineered file headers, accessors, and serializers for various NintendoWare and Nintendo EPD formats
* Yaz0 decompressor for Nintendo szs files and Zstd wrapper
* Common hash algorithms, crc32b, murmur3, and murmur64
* Common utilities
  * Scope Guard
  * Structured Error Handling
* Common Data Structures
  * Red Black Tree (Map)
  * Ring Buffer (circular queue)
  * Handle Table (index allocator)
  * Intrusive List (Bi-directional Linked List)
  * Priority Queue
  * Hash Map
  * Dependency graph (connection mask)
  * Pointer Array
  * Frame Array
  * Heap Array
  * Fixed String

# Awn Features
* An asynchronous file resource manager
* A cooperative thread manager
* A heap-tree based CPU and GPU memory manager
* Graphics wrapper api
* Basic mouse and keyboard support
* Base framework for a mainloop with a single window

# In Development
* A parameter system that can autogenerate byaml parameter accessors
* An AI behavior tree system
* A revamped multi-threaded framework based on dependency graphs
* An Actor and Scene system
* A UI and Font system utilizing the reverse engineered bflyt format

# System Requirements
* The user's Vulkan implementation must conform to all the limits and features enforced by awn::gfx::Context which include but are not limited to:
  * Device Addresses
  * Descriptor Buffers
  * Shader Objects
  * Resizable-Bar
* Requires an x86_64 CPU with AVX2, PCLMUL, RDTSC, and POPCNT

# Footnote
Built for Windows 10 with GCC 14.1 and gnu make from MSYS2. 
Build instructions:
1. Install MSYS2 
2. From MSYS2 install the standard GCC package
3. Run the newly installed Make program from the command line in the root project directory
4. Use the relevant libraries in other programs.

I'm informed by reverse engineering, text books, free online resources, and api documentation. I believe to be conformant with respect to my references.

If there are problems of any kind please file an issue or contact me and I'll do my best to resolve the issue.
