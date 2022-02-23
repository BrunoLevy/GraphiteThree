
# Geogram Pluggable Software Modules

Some users of Geogram may be interested only in a subset of
Geogram functionalities.
Following the principle that Geogram should be as easy to use/compile as
possible, some subsets of functionalities are alternatively available as
a standalone pair of (header,implementation) files, automatically
extracted/assembled from Geogram sourcetree. This makes the functionality
usable with 0 dependency: client code that uses a PSM just
need to insert the header and the implementation file into the project
(rather than linking with the entire Geogram library).

Geogram PSMs are available in the
[Geogram distribution](https://gforge.inria.fr/frs/?group_id=5833)

They can be also automatically generated from the Geogram sourcetree, by using:

    tools/extract_psm.sh filename.psm

on a Linux box or under Cygwin (the latter is untested).

Most PSMs come with an example file. To compile, use:

    g++ MultiPrecision_example.cpp MultiPrecision_psm.cpp -o MultiPrecision_example

(where MultiPrecision can be replaced with the PSM name)

## List of PSMs

### OpenNL 

(src/lib/geogram/NL/OpenNL.psm): solvers for sparse
linear systems / least squares regression

To compile, it needs the math library:

    gcc OpenNL_example.c OpenNL_psm.c -o OpenNL_example -lm 

To enable support for SuperLU dynamic linking (Linux only), use:

    gcc -DGEO_DYNAMIC_LIBS OpenNL_example.c OpenNL_psm.c -o OpenNL_example -lm -ldl

To enable OpenMP parallel mode, use:

    gcc -fopenmp OpenNL_example.c OpenNL_psm.c -o OpenNL_example -lm 

### MultiPrecision 

(src/lib/geogram/numerics/MultiPrecision.psm):
an easy-to-use number type for computing the exact sign of a polynom

To compile, use:

    g++ MultiPrecision_example.cpp MultiPrecision_psm.cpp -o MultiPrecision_example

### Predicates 

(src/lib/geogram/numerics/Predicates.psm):
exact geometric predicates with symbolic perturbation (SOS)

To compile, use:

    g++ -c Predicates_psm.cpp 

(no example program for now)


### Delaunay

(src/lib/geogram/delaunay/Delaunay.psm): Sequential and parallel Delaunay 
triangulation and power diagrams in 2D and 3D. 

To compile, use:

    gcc -O3 -fopenmp -frounding-math -ffp-contract=off Delaunay_example.cpp Delaunay_psm.cpp -o Delaunay_example -lm 



## Syntax of a PSM file

A PSM file specifies a list of header files and a list of
source files that will be assembled into a single header file
(respectively a single source file). All file paths are relative
to where the PSM file is located:

    HEADERS="header_1.h header_2.h ... header_n.h"

Sources can be either C++ or C files. If sources are only C files,
then the generated source is a C file, else it is a C++ file:

    SOURCES="source_1.cpp source_2.cpp ... source_n.cpp"

To avoid including too many Geogram classes in a PSM, there is a drop-in
replacement of some Geogram mechanisms (Logger, etc...) in basic/psm.h.

There is also a link to a documentation page, that is inserted
in the generated header:

    DOC="http://alice.loria.fr/software/geogram/doc/html/link to the doc.html"

Optionally, there can be an example file, that will be also extracted.
It can be a C++ or a C program. The specified path is relative to where
the PSM file is located:

    EXAMPLE="path to example.cpp"

