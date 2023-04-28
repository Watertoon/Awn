# Awn
Very experimental refactor/reorganization of: https://github.com/Watertoon/DD-Framework

Awn is my personal C++ library. Designed for real-time applications.

# Current Features
* A cooperative M:N threading environment powered by a usermode fiber scheduler
* A custom CPU and GPU heap memory manager
* A Vulkan 1.3 wrapper library targeting ampere GPUs, taking advantage of resizable bar, dynamic rendering, buffer device addresses, descriptor indexing, descriptor buffers, mesh shaders, and shader objects
* A custom texture and sampler resource binding model
* A file device and resource manager with support for certain reverse engineered Nintendo file formats
* A SIMD math library (Vectors, Matrices, Camera's, Projections)
* Many utility data structures (red black trees, intrusive lists, time, non-cryptographic hash algorithms, results, etc)
* A structured error handler
* Basic keyboard and mouse support

# Pending integration (in-drafting)
* Window support
* A font drawer
* A priority queue
* A dependency based multithreaded job queue system
* More reverse engineered Nintendo file format structure definitions

# Future Features (in early development/planning)
* More cryptograhically secure hash algorithms (SHA256)
* A file decompressor
* A parameter system based on Nintendo's BYAML file format
* An asynchronous file loader
* An environment built on the Vulkan wrapper library for managing 3d models and graphics techniques

# Project Stipulations
* I wish to limit the usage of external dependencies when possible. I currently rely on Win32 and Vulkan 1.3.
* The project should be simple to build beyond setting up MSYS2, installing the relevant MSYS2 packages, and running make.
* The user's Vulkan implementation must conform to all the limits and features defined in awn::gfx::Context.
* The project curently requires an x86_64 CPU with AVX2, PCLMUL, RDTSC, and POPCNT.

# Footnote
Built for Windows 10 with GCC 12.2 and gnu make from MSYS2. 
Build instructions:
1. Install MSYS2 
2. From MSYS2 install the standard GCC package
3. Run the newly installed Make program from the command line in the root project directory
4. Use the relevant libraries or run the relevant project exes.

I'm informed by learnopengl.com, reverse engineering, text books, free online resources, and documentation. I am not an expert on liscensing but I believe to be conformant with respect to my references.

If there are any problems of any kind please file an issue or contact me and I'll do my best to resolve the issue.