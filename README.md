# Awn
Awn is a personal C++ library designed to power low-latency multi-threaded real-time applications.

# Vp
Vp is an included personal C++ library designed to carry common utilities and file format definitions in a more portable form than Awn.

# Current Features
* A cooperative M:N threading environment powered by a usermode fiber scheduler
* A multithreaded job queue system enabling a Task Graph architecture
* An asynchronous task queue system enabling async job processing
* A custom CPU and GPU heap memory manager
* A Vulkan 1.3 wrapper library targeting ampere-level GPUs, taking advantage of: 
  * Resizable-Bar
  * Dynamic Rendering
  * Buffer Device Addresses
  * Descriptor Indexing
  * Descriptor Buffers
  * Mesh Shaders
  * Shader Objects
* A custom texture and sampler resource binding model
* A file device and resource manager with support for certain reverse engineered file formats
* A SIMD linear-algebra math library (Vectors, Matrices, Camera's, Projections)
* Many utility data structures (red black trees, intrusive lists, priority queue, time, non-cryptographic hash algorithms, results, etc)
* Structured error-handling
* Basic keyboard and mouse support
* Window support

# Pending integration (in-drafting)
* A pipeline for compiling shaders into a suitable container
* A font drawer
* A file decompressor
* More cryptograhically secure hash algorithms (SHA256)
* A parameter system based on Nintendo's BYAML file format

# Future Features (in early development/planning)
* An asynchronous file loader
* An environment for managing 3d models and graphics techniques

# Project Stipulations
* I prefer to limit the usage of external dependencies when possible.
* The project should be simple to build beyond setting up MSYS2, installing the relevant MSYS2 packages, and running make.
* The user's Vulkan implementation must conform to all the limits and features enforced by awn::gfx::Context.
* The project curently requires an x86_64 CPU with AVX2, PCLMUL, RDTSC, and POPCNT.

# Footnote
Built for Windows 10 with GCC 13.2 and gnu make from MSYS2. 
Build instructions:
1. Install MSYS2 
2. From MSYS2 install the standard GCC package
3. Run the newly installed Make program from the command line in the root project directory
4. Use the relevant libraries in other programs.

I'm informed by learnopengl.com, reverse engineering, text books, free online resources, and documentation. I am not an expert on liscensing but I believe to be conformant with respect to my references.

If there are any problems of any kind please file an issue or contact me and I'll do my best to resolve the issue.
