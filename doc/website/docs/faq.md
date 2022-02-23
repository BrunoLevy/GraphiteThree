# Frequently Asked Questions

### 1. What changed between Graphite][ and Graphite/// ?

- Graphite/// is now based on the new Geogram library. Geogram
implements the thin OS abstraction layer (FileSystem and Process),
an efficient mesh data structure (for surfacic, tetrahedral and
hybrid volumetric meshes), computational geometry (Kd-Tree, AABB-Tree,
parallel Delaunay triangulation, Voronoi diagrams, robust predicates
in arbitrary precision). 

- This means the representation of the objects completely changed,
there is now just a Mesh class in Geogram, that can represent
pointsets, polygonal lines, surfacic meshes and volumetric meshes.
Mesh is inherited by MeshGrob (i.e. a Mesh that can be used as
a Graphite object).

- We removed the RenderingPipeline / RenderingContext abstraction
layer. We now directly talk to OpenGL (it's simpler, standard
and much much faster).

- Code quality standards are higher: a higher warning level is
activated on all platforms, and we make our best to correct all
warnings for all architectures / all compilers. Geogram is completely
documented (Doxygen). Graphite documentation is under work.

- A new version of the GOMGEN compiler, that parses header files to
generate meta-information. It is used for instance to automatically
generate the GUI from the header files (reduces programmer's effort
to the minimum). The new version is based on Swig 3.0.7. It better
understands nested types (e.g. nested enums can now appear in the GUI).
I also hacked it to make it understand Doxygen comments (and automatically
insert Doxygen documentation in the GUI, as tooltips).

- A completely rewritten set of CMakeLists, that makes it much easier to
compile on various machines.

### 2. Where is the 2D editor ?

- We removed it, it was adding too much complexity for not much gain.
  To replace it, there is the "Param" shaders that display
  the objects in parameter-space.

### 3. I have several plugins that depend on Graphite][, does this mean I need to throw them away ?

- We are currently developing the 'legacy' plugin, a 'jumbo' plugin that
implements all the Graphite][ objects for Graphite/// (current state:
the library 'cells' was completely ported, we are proceeding to
'surfaces' and 'volumes', stay tuned...)

### 4. Under windows, can I use CMake 32 bits for compiling Graphite/// in 64 bits ?

- Yes, there is no problem doing that.
  It used to be a problem with Graphite][, that was using a complicated mechanism
in CMake to compute GOMGEN dependencies (it was generating a DLL extension for CMake),
the new mechanism uses plain standard CMake commands.

### 5. Why do you write Graphite][ and Graphite/// like that ?

- I am a big fan of Apple (in the pre-Macintosh period !)

### 6. What about Python and LUA ?

Since Graphite 1.4.4, the Graphite Embedded Language uses LUA as the runtime instead of
Python (Graphite 1.4.3 supports both runtimes). Why we did that ? Because we can do with
LUA everything that we were doing before with Python, and with a much simpler runtime, as
well as a much simpler glue code that interfaces the LUA interpreter with Graphite Object model.
Another reason is that Python has a special memory allocator that prevents Valgrind for
operating properly. With LUA, we can test Graphite for memory errors using Valgrind. Finally,
with LUA, the total size of a binary distribution was cut in half (12.5 Mb instead of 26 Mb).
LUA is more compatible with Geogram/Graphite design philosophy/minimalism ("small is beautiful").

