# Tutorial for basic finite element method 


## Introduction

This tutorial is about building a Graphite plugin to solve the Poisson equation
with the finite element method (FEM) on tetrahedral meshes. This is a very
simple implementation that only uses linear finite elements (P1).

This tutorial focuses on learning how to use the *Geogram and Graphite classes and functions*.
In particular, we will look at the following:

- GEO::Mesh class: import/export, vertices, facets, cells, local and global indexing, adjacencies
- surface reconstruction from point cloud
- smooth remeshing of surfaces
- tetrahedral meshing of closed surfaces with [TetGen](http://wias-berlin.de/software/tetgen/) (included in Geogram)
- linear algebra with OpenNL (included in Geogram)
- GEO::Attribute: attach values to mesh entities, to define piecewise-linear fields
- GEO::Logger: to output useful information, warnings, errors
- automated generation of commands in the Graphite GUI (transparent, done by `gomgen`)
- visualization of meshes and scalar fields in Graphite
- automated runs via Lua scripting

## Creation of the plugin

Launch `Graphite` and run the command *create plugin*, with *femb* as plugin
name in argument, in the `devel` menu. It generates a folder `femb` in the
directory containing all the `Graphite` plugins.

To tell `Graphite` to use the new *femb* plugin at start, go to *Files -> preferences...*
and add *femb* in the plugin list, then save the configuration file.

In a plugin folder, things are organized as follow:
- Algorithms (FEM, mesh pre/post-processing, etc) in the `algo/` directory
- Commands that can be called from the GUI or from Lua scripts in `commands/`
- Third-party libraries in `third_party/`

A good practice is that the code in `algo/` only depend on *Geogram* and does
not use *Graphite* features or classes (no include of *Graphite* headers).  In
the same spirit, the implementation of the commands in `commands/` should
contain very little algorithms and mainly act as a wrapper of functions defined
in the `algo/` folder.


## Mathematical functions

To solve (visually) interesting problems, we will need to be able define non-trivial math functions
at run-time. We will use them to define the boundary regions, diffusion coefficient and source term.
The simplest choice is to use the very good [ExprTK](https://github.com/ArashPartow/exprtk) library
which consists of only one header. It allows to parse and evaluate complicated functions at
run-time such as

    (sin(x / pi) cos(2y) + 1) / (sin(x / pi) * cos(2 * y) + 1)

See the official ExprTK website for the full list of supported functions.

To include the library in your project, just copy the header in a third party folder:

    cd path/to/plugin
    mkdir third_party
    wget -O third_party/exprtk.hpp https://raw.githubusercontent.com/ArashPartow/exprtk/master/exprtk.hpp


ExprTK is a fully templated library with slow compile time and as we will only
need functions from `R^3` to `R`, we can write a simple wrapper in a file that
will only be compiled one time.  You can directly retrieve the files
`algo/functions.h` and `algo/functions.cpp` or copy paste them from the code in 
[appendix](#appendix-exprtk-simple-wrapper).


## Graphite command

Graphite *commands* that act on the mesh currently selected in *Graphite* are
member methods of a class deriving from the base class `MeshGrobCommands`. To
create a new class of commands, we use the *create commands* function in the
`devel` menu. This command takes the plugin name (femb), the type of objects
(`OGF::MeshGrob`) and a name (we use *Fem*) as arguments. It generates a pair of
files `commands/mesh_grob_fem_commands.h/cpp` with a base implementation of
the new command class. For each function added in this new `MeshGrobFemCommands`,
there will be a widget automatically generated to call the function from the GUI and
a Lua binding so the function can also be called from scripts.

To get a quick feedback in the development cycle of a new plugin, it is usually
recommended to start by writing commands early to test the functionalities as
soon as possible.

We start by adding the main command of our finite element plugin:

    gom_class femb_API MeshGrobFemCommands : public MeshGrobCommands {
    public:
        MeshGrobFemCommands() ;
        virtual ~MeshGrobFemCommands() ;

    gom_slots:
        /**
         * \menu /FEM
         */
        bool fem(
                const std::string& dirichlet_region = "1",
                const std::string& dirichlet_value  = "0",
                const std::string& neumann_region   = "0",
                const std::string& neumann_value    = "0",
                const std::string& sourceterm_value = "1",
                const std::string& diffusion_value  = "1",
                const std::string& solution_name    = "u"
                );
    } ;

The menu comment is parsed by `gomgen` . It determines where the command will appear in
the Graphite GUI. All arguments must have a default value.

The implementation is mainly a wrapper to our finite element code that will be implemented
in `algo/femb.cpp`. In `MeshGrobCommands` derived class, there are two important functions:
`mesh_grob()` returns a pointer of type `MeshGrob` pointing to the current mesh (on which
the command has been called in the GUI or script) and `scene_graph()` returns a pointer
to the scene (which contains all the `MeshGrob` objects).

To avoid modifying the current mesh, we create a new mesh in the scene and we call the
finite element code on it:

    void MeshGrobFemCommands::fem(
            const std::string& dirichlet_region,
            const std::string& dirichlet_value,
            const std::string& neumann_region,
            const std::string& neumann_value,
            const std::string& sourceterm_value,
            const std::string& diffusion_value,
            const std::string& solution_name) {

        /* Copy current mesh to a new one for the FEM simulation */
        std::string Mo_name = mesh_grob()->name() + "_fem";
        MeshGrob* Mo = MeshGrob::find_or_create(scene_graph(), Mo_name);
        Mo->copy(*mesh_grob());

        femb::fem_simulation(*Mo, dirichlet_region, dirichlet_value,
                neumann_region, neumann_value, sourceterm_value,
                diffusion_value, solution_name);

        Mo->update();
    }

Do not forget to call `update()` of meshes modified by the commands to update the graphics
in the GUI.

The *Graphite* part of our plugin is ready. We can now focus on the finite element code
and test it quickly by running the `fem(..)` command in the GUI. You can also write a
simple Lua script that run the command at start:

    -- Lua script to run fem on the current mesh: 
    scene_graph.current().query_interface("OGF::MeshGrobFemCommands").fem({sourceterm_value="1", dirichlet_region="1", neumann_value="0", solution_name="u", diffusion_value="1", dirichlet_value="0", neumann_region="0"})

Now from the command line, you can open Graphite with a mesh and automatically execute this script:

    /path/to/graphite/executable /path/to/mesh.mesh /path/to/script.lua


## FEM code

All the FEM code is in the function `fem_simulation(...)` implemented in the file `algo/femb.cpp`.

    bool fem_simulation(GEO::Mesh& M,
            const std::string& dirichlet_region,
            const std::string& dirichlet_value,
            const std::string& neumann_region,
            const std::string& neumann_value,
            const std::string& sourceterm_value,
            const std::string& diffusion_value,
            const std::string& solution_name
            );

The string values define the Poisson problem and the geometry is given by the GEO::Mesh
(passed by an command-line executable or by a *command* defined in a file of the `commands/`
folder).
This code only depends on Geogram and does not use Graphite.

### a) Mesh pre-processing

For the finite element method algorithm, we need a valid tetrahedral mesh. If the input
is not such a mesh, we try to build one.
At the beginning of the `bool fem_simulation(...)` function, we add the necessary mesh 
pre-processing code.

If the input is a point cloud (no facets, no cells in `M`), we build a closed
triangulated surface to get a 3D model boundary.
If the input is triangulated surface without cells, we tetrahedralize the interior 
with `TetGen`. We also apply other cleaning functions to ensure consistency in the mesh
data structure (useful if the input had wrong connectivity or things like this).

    /* ------ MESH PREPROCESSING ------ */
    /* Closed surface from point cloud if no cells/facets */
    if (M.cells.nb() == 0 && M.facets.nb() == 0 && M.vertices.nb() > 0) {
        Logger::out("fem") << "building a closed surface from point cloud" << std::endl;
        double radius = 5.;    /* reconstruction parameter */
        double R = bbox_diagonal(M);
        mesh_repair(M, GEO::MESH_REPAIR_COLOCATE, 1e-6*R);
        radius *= 0.01 * R;
        Co3Ne_reconstruct(M, radius);
    
        /* Smooth remeshing of the reconstructed surface */
        unsigned int nb_points = M.vertices.nb() * 2;
        unsigned int Lloyd_iter = 5;
        unsigned int Newton_iter = 30;
        unsigned int Newton_m = 7;
        Mesh M_tmp(3); /* because output of remeshing is a different mesh*/
        remesh_smooth(M, M_tmp, nb_points, 0, Lloyd_iter, Newton_iter, Newton_m);
        M.copy(M_tmp); /* result of remeshing is copied in initial mesh */
    }
    
    /* Try to mesh the volume with TetGen */
    if (M.cells.nb() == 0 && M.facets.nb() > 0) {
        /* Triangulate surface mesh (useful if input is quad or polygonal mesh) */
        if (!M.facets.are_simplices()) {
            M.facets.triangulate();
        }
    
        /* Colocate vertices (useful if input is STL mesh) */
        Logger::out("fem") << "colocating vertices of triangulated mesh" << std::endl;
        double R = bbox_diagonal(M);
        mesh_repair(M, GEO::MESH_REPAIR_COLOCATE, 1e-6*R);
    
        /* TetGen call */
        Logger::out("fem") << "tetrahedralize interior of triangulated mesh" << std::endl;
        mesh_tetrahedralize(M, false, true, 0.7);
    }
    
    /* Verify input */
    if (!(M.cells.nb() > 0 && M.cells.are_simplices())) {
        Logger::out("fem") << "invalid input mesh" << std::endl;
        return false;
    }
    
    /* Ensure right adjacencies and matching between volume (cells) 
     * and boundary (facets).
     * Useful if input is tet mesh without or with bad boundaries */
    M.cells.connect();
    M.facets.clear();
    M.cells.compute_borders(); /* add facets to M on the boundary */
    M.vertices.remove_isolated(); /* avoid hanging vertices in the mesh */

At this point, we should have a valid tetrahedral mesh for simple linear FEM.

### FEM and linear system assembly

The Poisson problem is defined by:
$$ - div ( c \nabla u) = f \text{ in } \Omega, \quad u = u_d \text { on } \partial \Omega_D, \quad \nabla u \cdot n = g_n \text{ on } \partial \Omega_N $$

We use the standard weak-form: 

$$ \int_\Omega c \nabla u \cdot \nabla v = \int_\Omega f v + \int_{\partial \Omega_N} g_n v $$

After FEM discretization on a $(\phi_i)_{i=1..n}$ basis of piecewise-linear functions (defined
on the tets of the mesh `M`), we got the linear system:

$$ A x = B $$

$$ a_{ij} = \sum_K \int_K c \nabla \phi_i \cdot \nabla \phi_j, \quad b_i = \sum_K \int_K f \phi_i + \sum_{\partial K_n} \int_{\partial K_n} g_n \phi_i $$

After change of variable (mapping $M$) and quadratures (weights and points $w_k,\hat x_k$):

$$
a_{ij} = \sum_{k=0}^{p-1} w_k \  c(M(\hat x_k)) \ (J^{-1})^T\nabla\hat{\phi}_i(\hat x_k) \cdot (J^{-1})^T\nabla\hat{\phi}_j(\hat x_k) \ \text{det} J(\hat x_k)
$$

$$
b_{i}^{vol} =  \sum_{k=0}^{p-1} w_k \hat{\phi}_i(\hat x_k)f(M(\hat x_k)) \ \text{det}J(\hat x_k)
$$

(corriger celui d'après !! TODO)
$$
b_{i}^{bdr} = \sum_{k=0}^{p-1} w_k  \hat{\phi}_k(\hat x) \ g_n(M(\hat x)) \ J^T J
$$

So for each tet `K` of the mesh `M` and for each triangle `dK` of the boundary, we need
to compute the contributions to the sparse matrix A and the vector B.


### FEM and linear system assembly

### Solve


## Appendix ExprTK simple wrapper 

    // Header: algo/functions.h
    #pragma once
    #include "OGF/femb/third_party/exprtk.hpp"
    #include <string>
    
    namespace femb {
        class ScalarFunction {
            public:
                ScalarFunction(const std::string& expr);
                double operator()(const double point[3]);
    
            protected:
                exprtk::expression<double>      expr_;
                exprtk::parser<double>          parser_;
                exprtk::symbol_table<double>    symbol_table_;
                double                          pt_[3];
        };
    }

    // Implementation: algo/functions.cpp
    #include "OGF/femb/algo/functions.h"
    
    namespace femb {
        ScalarFunction::ScalarFunction(const std::string& expr){
            symbol_table_.add_variable("x", pt_[0]);
            symbol_table_.add_variable("y", pt_[1]);
            symbol_table_.add_variable("z", pt_[2]);
            symbol_table_.add_constants();
            expr_.register_symbol_table(symbol_table_);
    
            if(!parser_.compile(expr,expr_) ) {
                std::cout << "[exprtk parsing] Wrong expression: \n "
                    << parser_.error() << std::endl;;
                return;
            }
        }
    
        double ScalarFunction::operator()(const double point[3]){
            pt_[0] = point[0];
            pt_[1] = point[1];
            pt_[2] = point[2];
            return expr_.value();
        }
    }

