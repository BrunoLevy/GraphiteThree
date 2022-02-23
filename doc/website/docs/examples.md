# Graphite

Graphite commands are listed in the GUI menus (e.g. right-click on a mesh of scene object list).
You can look at the videos in the [gallery](gallery.md) to see some of them in use.

# Geogram sample programs

This is a list of executables built by default when compiling *Geogram*.

The options for all example programs can be queried using <program_name> -h or <program_name> /?

## vorpaview 
a simple mesh viewer

Usage: 

    vorpaview meshfile

where meshfile is a .mesh, .meshb, .ply, .tet, .obj or .stl file.

If the graphic board supports GLSL, vorpaview uses shaders. If the
graphic board does not have sufficient GLSL support, vorpaview uses
plain OpenGL (fixed functionality pipeline). Note that some old
OpenGL drivers may erroneously claim they support GLSL. If you have
this problem, you can force vorpaview to deactivate GLSL mode, use:

    vorpaview gfx:GLSL=false meshfile 

Alternatively, you can rename vorpaview(.exe) as vorpaview0(.exe),
this will deactivate GLSL by default. Note that some options / key bindings
may not work under this degraded mode. 

The key bindings are mostly the same as in the 'medit' mesh viewer
(see below). For instance, F1, F2 and F3 control the clipping planes.
Additional key bindings can be found using 'h', that toggles the online help. 

## vorpastat
vorpastat measures the Hausdorff distance between two meshes 

Usage: 

    vorpastat meshfile1 meshfile2

vorpastat can be used to automatically test remeshing algorithms on mesh databases.
It tests both mesh topology and Haussdorff distance. Whenever the mesh topologies differ
or the Haussdorff distance is larger than 5% of the bounding box diagonal, it
returns a non-zero result. 

    vorpastat stat:sampling_step=nnn meshfile1 meshfile2 

can be used to specify the average distance between
two samples used to estimate the Hausdorff distance (expressed in absolute units or
in percentage of the bounding box diagonal if it has a trailing '\%' sign).

## vorpalite

vorpalite implements several reconstruction and remeshing algorithms.
Use the following command to see all the available options:

    vorpalite -h

Some examples: 

Reconstruction (clean point cloud):

    vorpalite profile=reconstruct my_points.xyz my_surface.meshb

Reconstruction (noisy point cloud):

    vorpalite profile=reconstruct co3ne:Psmooth_iter=2 my_points.xyz my_surface.meshb

Remeshing (anisotropic):

    vorpalite profile=remesh input_surface.meshb output_surface.meshb pts=30000

where 30000 is the desired number of points

Remeshing (isotropic):

    vorpalite profile=remesh input_surface.meshb output_surface.meshb pts=30000 aniso=0

Remeshing (graded):

    vorpalite profile=remesh input_surface.meshb output_surface.meshb pts=30000 aniso=0 gradation=1

Repair:

    vorpalite profile=repair input_surface.meshb output_surface.meshb

Tetrahedral meshing (through bundled TetGen, by Hang Si):

    vorpalite profile=tet input_surface.meshb output_tetrahedral_mesh.meshb

##compute_delaunay
computes the 3D Delaunay triangulation of a set of points. Note: this is not a
reconstruction method, the 3D Delaunay triangulation is a tetrahedral mesh that
uses all the points as its vertices, and that has a boundary that corresponds to
the convex hull of the pointset. It is not directly useful for applications, but some users
may want to compute it to extract information from it, e.g. to implement their own
reconstruction method.

usage:

    compute_delaunay pointsfile <outputfile>

The points file can be in any mesh format supported by Geogram (.mesh, .meshb, .ply ...)
and in .xyz file format (ASCII, one point per line). The output file (default: out.meshb
if not specified) contains the tetrahedra. The result can be displayed using vorpaview or medit.

Additional parameters:

- algo:delaunay=(BDEL|PDEL|tetgen) specify the implementation of the Delaunay triangulation. It can be one of
  PDEL:Geogram parallel (default), BDEL:Geogram sequential or tetgen (if compiled with tetgen support).
- dbg:delaunay_benchmark=true to enable more statistics/timings
- sys:stats=true to display statistics (including predicates and symbolic perturbation)


## compute_OTM
computes semi-discrete optimal transport map in 3D,
The algorithm is described in the following references:

1. 3D algorithm: http://arxiv.org/abs/1409.1279
2. Earlier 2D version by Quentin Merigot: 
 Q. Merigot. A multiscale approach to optimal transport.
 Computer Graphics Forum 30 (5) 1583--1592, 2011 (Proc SGP 2011).
3. Earlier theoretical article on OT and power diagrams: 
 F. Aurenhammer, F. Hoffmann, and B. Aronov. Minkowski-type theorems 
 and least-squares clustering. Algorithmica, 20:61-76, 1998.

usage:

    compute_OTM meshfile1 meshfile2 

where meshfile1 and meshfile2 are volumetric tetrahedral meshes. If geogram is compiled
with tetgen support (see \link geogram_compiling  "compiling" \endlink), mesh files
can be surfacic meshes (they are then tetrahedralized using tetgen).

This computes a morphing between meshfile1 and meshfile2, saved in morph.tet6, and that
can be displayed using vorpaview (see above). In addition to standard vorpaview key
bindings, one can use 't' and 'r' to navigate through time and animate the morphing.
'F1' can be used to display the tetrahedra inside the mesh.

Additional parameters

- recenter = true: moves the second mesh in such a way that their centroids match
- rescale = true: rescale the second mesh in such a way it has the same volume as the first one
- nb_pts = nnnnn: number of points used to resample the second mesh (default = 1000, use typically
 30000 for a nice result)
- singular = true: output the "singular set" (i.e. the points where transport is discontinuous) to
 singular.obj
- out=filename.eobj|filename.obj6|filename.tet6: specify the output file. tet6 file format is a
 tetrahedral mesh with 6 coordinates per vertex (original and final time). obj6 file format is
 a surfacic mesh with 6 coordinates per vertex (original and final time), and eobj file format
 is a surfacic animated mesh with additional information (Kantorovich potential), to be displayed
 in the Graphite software.


## compute_RVD
computes the intersection between a Voronoi diagram and a surfacic or volumetric simplicial set

