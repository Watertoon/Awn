# Awn
Awn is a personal C++ library designed to power low-latency multi-threaded real-time applications.

# Vp
Vp is an included personal C++ library designed to carry common utilities and file format definitions in a more portable form than Awn.

# Features
* A usermode fiber scheduler using an M:N threading model to create a cooperative usermode thread scheduler
* A multi-threaded dependency-based job graph system for bulk processing
* An asynchronous task system for background processing
* A custom CPU and GPU heap system to manage memory
* A Vulkan 1.3 wrapper library targeting ampere-level GPUs, featuring:
  * A commitment to managing GPU memory through Virtual Addresses outside texture data via Resizable Bar
  * A commitment to using shader objects for flexible and dynamic setting of fixed function state
  * A custom TextureSampler manager for managing binding descriptor slots and texture memory
* A file device and resource manager with support for file formats reverse engineered and adapted from predominantly Nintendo EPD games
* A SIMD linear-algebra math library
  * Vector api
  * Matrix api
  * Camera api
  * Projection api
* Basic keyboard and mouse support
* Basic window framework
* Many utility data structures
  * IntrusiveList providing an interface for creating 2 way lists out of structures
  * IntrusiveRedBlackTree providing an interface for sorted maps out of structures
  * HandleTable providing an interface for bookkeeping the existence of objects via handles
  * Runtime Type Info api
  * Atomics api
  * Time api
  * ScopeGuard providing RAII style scope cleanup
  * Functional error-handling via Result

# System Requirements
* The user's Vulkan implementation must conform to all the limits and features enforced by awn::gfx::Context which include but are not limited to:
  * Device Addresses
  * Descriptor Buffers
  * Shader Objects
  * Resizable-Bar
* Requires an x86_64 CPU with AVX2, PCLMUL, RDTSC, and POPCNT

# Footnote
Built for Windows 10 with GCC 13.2 and gnu make from MSYS2. 
Build instructions:
1. Install MSYS2 
2. From MSYS2 install the standard GCC package
3. Run the newly installed Make program from the command line in the root project directory
4. Use the relevant libraries in other programs.

I'm informed by reverse engineering, text books, free online resources, and api documentation. I believe to be conformant with respect to my references.

If there are problems of any kind please file an issue or contact me and I'll do my best to resolve the issue.
