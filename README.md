# Awn
Awn is a personal C++ library designed to power low-latency multi-threaded real-time applications. It's based on common game literature and reverse engineerings of retail game's.
## Sub-libraries

| sub-library | description                                                                                                                                                                                |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| mem         | Virtual memory and heap memory management library.                                                                                                                                         |
| sys         | Thread and system environment management library for the supported underlying Operating System.                                                                                            |
| async       | Library for multi-threaded workers. Implements Asynchronous tasks, and dependency-based job graphs.                                                                                        |
| res         | Resource library. File devices handle file systems. Resources handle per-file data. An asynchronous resource management system is provided for real-time data streaming and decompression. |
| gfx         | A high-level graphics api based on modern Vulkan extensions, as well as reverse engineered nn::gfx and NVN.                                                                                |
| para        | A parameter library for generating parameter classes, handling resource databases, collaborating agents with blackboards, etc.                                                             |
| hid         | Host input device library. Supports keyboard and mice input reporting.                                                                                                                     |
| grr         | A graphics rendering library. Implements a display server and an immediate mode drawer.                                                                                                    |
| frm         | Bootstraps libraries.                                                                                                                                                                      |

## In Development

| sub-library | in-progress description                                                  |
| ----------- | ------------------------------------------------------------------------ |
| grr         | Immediate mode rendering, font rendering, model rendering, ui rendering. |
| act         | An actor library based on the reverse-engineering of shaft::baseproc.    |
| hid         | Bluetooth api. Game controller support for major platforms.              |
| audio       | Audio device output, input, and basic rendering.                         |
| frm         | A modular real-time application main-loop and an event-based main-loop.  |
| ai          | A behavior tree system based on the reverse-engineering of ainb.         |

# Vp
Vp is a personal C++ library designed to portably carry common utilities and file format definitions.
## Sub-libraries

| sub-libraries | description                                                                                                                  |
| ------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| codec         | Compression and decompression codec library, supports zstd.                                                                  |
| image         | Image format library, supports PNG encoding and decoding through libpng.                                                     |
| imem          | A heap allocator interface.                                                                                                  |
| resbui        | File format serialization library. Contains builders for some reverse engineered NintendoWare and Nintendo EPD file formats. |
| res           | File format library. Contains many reverse engineered NintendoWare and Nintendo EPD file format headers.                     |
| trace         |                                                                                                                              |
| util          | A large common data structure, algorithm, and utility sink.                                                                  |

## In Development

| sub-libraries | description                                                                               |
| ------------- | ----------------------------------------------------------------------------------------- |
| dsp           | A set of digital signal processing functions for pcm conversion, resampling, and effects. |

# System Requirements
>The user's Vulkan implementation must conform to all the limits and features described by the awn::gfx::RequiredFeature enum which include but are not limited to:
	>Vulkan 1.4
	>Descriptor Buffers
	>Mutable Descriptors
	>Host Image Copy
	>Shader Objects
	>Resizable-Bar
> Requires an x86_64 CPU with AVX2 and PCLMUL, or an AArch64 CPU with NEON

# Footnote
Built for Windows 10 with GCC 15.2 and gnu make from MSYS2. 
Build instructions:
1. Install MSYS2 
2. From MSYS2 install the standard GCC package
3. Run the newly installed Make program from the command line in the root project directory
4. Link the relevant libraries in other programs

I'm informed by reverse engineering, text books, free online resources, and public api documentation. I am not under NDA. If there are problems of any kind please file an issue or contact me and I'll do my best to resolve the issue.

This repository is currently an irregular snapshot of the project.

Licensed under GPLv2 only. Following Atmosphère's lead, due to my use of reverse-engineering, Nintendo is exempt from GPLv2 licensing, and at it's discretion may instead license any of the source code authored for the Awn or Vp projects under the Zero-Clause BSD license.
