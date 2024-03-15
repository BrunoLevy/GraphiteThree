
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the following 
 *  (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_WARPDRIVE_COMMANDS_MESH_GROB_TRANSPORT_COMMANDS_H
#define H_OGF_WARPDRIVE_COMMANDS_MESH_GROB_TRANSPORT_COMMANDS_H

#include <OGF/WarpDrive/common/common.h>
#include <OGF/WarpDrive/algo/velocity_field.h>
#include <OGF/WarpDrive/algo/time_integrator.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>
#include <exploragram/optimal_transport/optimal_transport.h>

namespace OGF {

    gom_class WarpDrive_API MeshGrobTransportCommands :
        public MeshGrobCommands {
    public:
        MeshGrobTransportCommands() ;
        ~MeshGrobTransportCommands() override;

    gom_slots:

        /**
	 * \menu Pre-processing
         * \brief Aligns two meshes before optimal transport.
         * \param[in] target the name of the target mesh
         * \param[in] recenter if true, recenter current on target
         * \param[in] rescale if true, rescale current in such a way
         *  it has the same volume as the target
         */
        void align(
            const MeshGrobName& target,
            bool recenter = true,
            bool rescale = true
        );

        /**
	 * \menu Pre-processing
         * \brief Creates varying densities for optimal transport.
         * \param[in] density1 minimum value of the density
         * \param[out] density2 maximum value of the density
         * \param[in] function specification of the function to be used,
         *  in the form "(+|-)?func(^pow)?", where func is one of 
         *  X,Y,Z,R,sin,dist
         * \param[in] reference if specified and func is "dist",
         *  distance is computed relative to this surface, else it
         *  is computed relative to the current surface
         */
        void set_density_by_formula(
            double density1 = 1.0,
            double density2 = 10.0,
            const std::string& function = "sin^2",
            const std::string& reference = ""
        );


        /**
	 * \menu Pre-processing
         * \brief normalize DESI data
         * \param[in] nb_bins for selection function
         * \param[in] subsample if non-zero, only use a subset of the data
         */
        void DESI_normalize_and_compute_selection_function(
            index_t nb_bins = 1000,
            index_t subsample = 0
        );
        
        /**
	 * \menu Compute
         * \brief Computes the optimal transport between this volumetric mesh
         *  and another one.
         * \param[in] target name of the target volumetric mesh 
         * \param[in] nb_points number of points to be created to 
         *  sample the target
         * \param[in] proj_border if true, points near the
         *  border are projected onto the boundary of the target
         * \param[in] result name of the computed morphing 
         * \param[in] Newton if true use Newton solver
         * \advanced
         * \param[in] sampling (optional) name of the computed target sampling
         * \param[in] singular_set (optional) compute the discontinuities
         * \param[in] regularization (optional) if non-zero, add regularization
         * \param[in] BRIO if true, use Biased Random Insertion
         *  Order [Amenta et.al]
         * \param[in] multilevel if true, use multilevel sampling 
         *  (note: BRIO implies multilevel)
         * \param[in] ratio ratio between the sizes of two sucessive levels
         * \param[in] nb_iter maximum number of iterations
         * \param[in] epsilon maximum relative measure error in a cell
	 * \param[in] linsolve_nb_iter the maximum number of iterations 
	 *   for linear solves
	 * \param[in] linsolve_epsilon the maximum value of 
	 *   \f$ \| Ax - b \| / \| b \| \f$ for linear solves
         * \param[in] save_RVD_iter if true, save all iterations 
         * \param[in] show_RVD_center if true and savev_RVD_iter, 
         *  shows center of cells attached to cells
         * \param[in] save_last_iter if true, save only last iteration
         */
        void transport_3d(
            const MeshGrobName& target,
            index_t nb_points = 10000,
            bool proj_border = true,
            const NewMeshGrobName& result = "morph",
            bool Newton = false,
            const NewMeshGrobName& sampling = "",
            const NewMeshGrobName& singular_set = "",
            double regularization=0.0,
            bool BRIO=true,
            bool multilevel=true,
            double ratio=0.125,
            index_t nb_iter = 1000,
            double epsilon = 0.01,
	    index_t linsolve_nb_iter = 1000,
	    double linsolve_epsilon = 0.001,
            bool save_RVD_iter=false,
            bool show_RVD_center=false,
            bool save_last_iter=false
        );


	/**
	 * \menu Compute
	 * \brief Computes Early Universe Reconstruction.
	 * \param[in] points the pointset
	 * \param[in] scaling_factor multiply all coordinates 
	 *   to improve stability
         * \param[in] Newton_epsilon maximum relative measure error in a cell 
	 * \param[in] max_Newton_iter maximum number of Newton iterations
	 * \param[in] linesearch_init_iter first iteration for linesearch
	 * \param[in] max_linesearch_iter maximum number of 
	 *   line search iterations
	 * \param[in] linsolve_epsilon tolerance for linear solve
	 * \param[in] max_linsolve_iter maximum number of 
	 *   linear solve iterations
	 * \param[in] regularization (optional) if non-zero, add regularization
	 */
	void early_universe_reconstruction(
            const MeshGrobName& points,
	    double scaling_factor = 10.0,
	    double Newton_epsilon = 1e-10,
	    index_t max_Newton_iter = 20,
	    index_t linesearch_init_iter = 10,
	    index_t max_linesearch_iter = 20,
	    double linsolve_epsilon = 1e-2,
	    index_t max_linsolve_iter = 1000,
	    double regularization = 0.0
	);

	
	/**
	 * \menu Compute
	 * \brief Computes a Laguerre diagram where the cells have all
	 *  the same areas
	 * \param[in] points the generated pointset 
	 * \param[in] laguerre the generated Laguerre diagram
	 * \param[in] air an optional pointset with air particles
	 * \param[in] air_fraction if air is specified, fraction of total
	 *  surface occupied by air particles
	 * \param[in] surface3D true if the surface is 3D, else 2D
	 * \param[in] nb_points number of points
         * \advanced
	 * \param[in] nb_iter number of Newton iterations
	 * \param[in] epsilon maximum relative measure error in a cell
	 * \param[in] regul regularization term
	 * \param[in] solver one of OT_PRECG, OT_SUPERLU, OT_CHOLMOD
	 */
	void isoarea_Laguerre_2d(
	    const NewMeshGrobName& points = "points",
	    const NewMeshGrobName& laguerre = "Laguerre",
	    const NewMeshGrobName& air = "",
	    double air_fraction = 0.5,
	    bool surface3D=false,
	    index_t nb_points = 1000,
	    index_t nb_iter = 1000,
	    double epsilon = 0.01,
	    double regul = 1e-3,
	    OTLinearSolver solver = OT_PRECG
	);
	
	
        /**
	 * \menu Compute
         * \brief Computes symetrized semi-discrete transport between 
         *   two objects.
         * \param[in] other the other object
         * \param[in] nb_points number of points
         * \param[in] thissampling sampling of this object
         * \param[in] othersampling sampling of the other object
         * \param[in] nb_iter number of symmetric transport iterations
         */
        void symmetric_transport_3d(
            const MeshGrobName& other,
            index_t nb_points=10000,
            const NewMeshGrobName& thissampling = "points1",
            const NewMeshGrobName& othersampling = "points2",
            index_t nb_iter=4
        );
        
        /**
	 * \menu Pre-processing
         * \brief Creates a regular sampling of the current object.
         * \param[in] sampling the name of the created object
         * \param[in] nb the number of samples per axis
         */
        void create_regular_sampling(
            const NewMeshGrobName& sampling = "points",
            index_t nb = 30
        );

        /**
	 * \menu Pre-processing
         * \brief Applies a random perturbation to all the vertices
         *  of a mesh.
         * \param[in] howmuch the magnitude of the perturbation
         */
        void perturb(double howmuch);

	enum EulerMode { EULER_2D=2, EULER_3D=3, EULER_ON_SURFACE=4};
	
	/**
	 * \menu Pre-processing
	 * \brief Moves points to barycenters of Laguerre cells
	 * \param[in] omega name of the domain
	 * \param[in] mode one of EULER_2D, EULER_3D, EULER_ON_SURFACE
	 */
	void init_Euler(
	    const MeshGrobName& omega,
	    EulerMode mode,
	    const NewMeshGrobName& air_particles="",
	    const NewMeshGrobName& fluid_omega0=""
	);


	/**
	 * \menu Pre-processing
	 * \brief Computes the density given by mass attribute and domain
	 *   volume.
	 */
	void get_density(const MeshGrobName& domain);

	/**
	 * \menu Pre-processing
	 * \brief Sets point masses from specified density and domain volume
	 */
	void set_density(const MeshGrobName& domain, double density);

	/**
	 * \menu Pre-processing
	 * \brief Append points and mass attribute
	 */
	void append_points(const MeshGrobName& points);
	
        /**
	 * \menu Compute
         * \brief Performs one step of incompressible Euler simulation in 2d
         * \param[in] omega the volumetric mesh with the domain
         * \param[in] nb_iter number of time steps
	 * \param[in] show_RVD show Laguerre diagram
	 * \param[in] show_centroids show centroids
	 * \advanced
         * \param[in] tau time step
         * \param[in] epsilon should be smaller than tau
         * \param[in] g gravity 
	 * \param[in] save_every save object every nnn
	 * \param[in] first_iter number of the first iteration,
	 *  used to name files correctly when doing multiple runs
	 * \param[in] verbose if set, displays status messages during
	 *  optimization.
	 * \param[in] project_every if non-zero, relocate each point to
	 *  the barycenter of its cell every nnn interations.
	 * \param[in] physical if true, then update using Newton second law,
	 *  else update as in initial article.
	 * \param[in] air_particles pointset with air particles if any
	 * \param[in] fluid_omega0 initial domain occupied by the fluid if any
	 * \param[in] physical true if using F=ma, else uses F=a
	 * \param[in] no_transport if true, just use non-optimized 
	 *  Voronoi diagram
         */
	void Euler2d(
            const MeshGrobName& omega,
            double tau=0.001,
            double epsilon=0.004,
            double g=10.0,
            index_t nb_iter=1,
	    index_t save_every=0,
	    index_t first_iter=0,
	    bool show_RVD=false,
	    bool show_centroids=false,
	    bool verbose=false,
	    index_t project_every=0,
	    const NewMeshGrobName& air_particles="",
	    const NewMeshGrobName& fluid_omega0="",
	    bool physical=true,
	    bool no_transport=false
	);

        /**
	 * \menu Compute
         * \brief Performs one step of incompressible Euler simulation in 3d
         * \param[in] omega the volumetric mesh with the domain
         * \param[in] nb_iter number of time steps
	 * \param[in] compute_interface if set, compute the interface between
	 *  fluides of different densities
	 * \param[in] split_interface if set, apply two steps of Catmull-Clark
	 *  subdivision to the interface, to make it smoother
	 * \advanced
         * \param[in] tau time step
         * \param[in] epsilon should be smaller than tau
         * \param[in] g gravity 
	 * \param[in] save_every save object every nnn
	 * \param[in] first_iter number of the first iteration,
	 *  used to name files correctly when doing multiple runs
	 * \param[in] verbose if set, messages are displayed during 
	 *  optimization.
	 * \param[in] project_every if non-zero, relocate each point to
	 *  the barycenter of its cell every nnn interations.
	 * \param[in] physical if true, then update using Newton second law,
	 *  else update as in initial article.
         */
        void Euler3d(
            const MeshGrobName& omega,
            double tau=0.001,
            double epsilon=0.004,
            double g=10.0,
            index_t nb_iter=1,
	    index_t save_every=0,
	    index_t first_iter=0,
	    bool compute_interface=false,
	    bool split_interface=false,
	    bool verbose=true,
	    index_t project_every=0,
	    bool physical=true
        );

        /**
	 * \menu Compute
         * \brief Performs one step of incompressible Euler 
	 *   simulation on surface
         * \param[in] omega the surfacic mesh with the domain
         * \param[in] nb_iter number of time steps
	 * \param[in] compute_RVD show Laguerre diagram
	 * \advanced
         * \param[in] tau time step
         * \param[in] epsilon should be smaller than tau
         * \param[in] g gravity 
	 * \param[in] verbose if set, messages are displayed during 
	 *  optimization.
	 * \param[in] project_every if non-zero, relocate each point to
	 *  the barycenter of its cell every nnn interations.
	 * \param[in] physical if true, then update using Newton second law,
	 *  else update as in initial article.
         */
        void Euler_on_surface(
            const MeshGrobName& omega,
            double tau=0.001,
            double epsilon=0.004,
            double g=10.0,
            index_t nb_iter=1,
	    bool compute_RVD=false,
	    bool verbose=false,
	    index_t project_every=0,
	    bool physical=true
        );

	
        /**
	 * \menu Pre-processing
         * \brief Generates a volumetric mesh that fills the space
         *  between the current mesh and an external shell.
         * \param[in] shell the shell
         * \param[in] inner_density density inside the object
         * \param[in] outer_density_max density on the part of the generated
         *  mesh that touches the object
         * \param[in] outer_density_min density on the part of the generated
         *  mesh that is furthest away from the object
         * \param[in] gamma exponent applied to the density
	 * \param[in] shell_only if set, do not generate tetrahedra inside
	 *  the current mesh.
         */
        void shell_mesh(
            const MeshGrobName& shell,
            double inner_density = 1.0,
            double outer_density_max = 1.0,
            double outer_density_min = 0.1,
            double gamma = 10.0,
	    bool shell_only = false
        );


	/**
	 * \menu Pre-processing
	 * \brief Keeps only the mesh elements that are inside
	 *   a certain domain.
	 * \param[in] domain the name of the domain mesh.
	 */
	void crop_domain(const MeshGrobName& domain = "omega");
	
	/**
	 * \menu Pre-processing
	 * \brief Keeps only the mesh elements that are inside
	 *   a certain region.
	 * \param[in] cell_center if true, cells that have their
	 *   centers in the region are kept, else only cells fully
	 *   included in the region are kept.
	 * \param[in] per_vertices if true, vertices that are outside
	 *   the region are deleted, as well as all mesh elements incident
	 *   to them.
	 */
	void crop_region(
	    double xmin = 0.0,
	    double ymin = 0.0,
	    double zmin = 0.0,
	    double xmax = 1.0,
	    double ymax = 1.0,
	    double zmax = 1.0,
	    bool cell_center = true,
	    bool per_vertices = false
	);

	/**
	 * \menu Compute
	 * \brief Fits a surface to another one using Voronoi Squared Distance
	 *  Metric algorithm.
	 * \param[in] affinity the higher, the smoother
	 * \param[in] nb_iter number of iterations
	 * \param[in] grid the object to be fitted or control mesh 
	 *  of subd surface
	 * \param[in] nb_subd number of subdivision
	 * \param[in] subd computed subdivision surface 
	 *  (if nb_subd different from 0)
	 */
	void optimize_VSDM(
	    double affinity=1.0,
	    index_t nb_iter=30,
	    const MeshGrobName& grid="grid",
	    index_t nb_subd = 0,
	    const NewMeshGrobName& subd="subd"
	);

	/**
	 * \menu Post-processing
	 * \brief Copies coordinates of another mesh into
	 *  a "t0" vector attribute.
	 * \param[in] t0_mesh the name of the mesh with 
	 *  geometry at t0 to be copied. Needs to have the
	 *  same number of vertices as this mesh.
	 */
	void copy_t0(
	    const std::string& t0_mesh
	);

	/**
	 * \menu Post-processing
	 * \brief Computes velocities from speed vectors.
	 * \param[in] velocity name of the attribute where to 
	 *  store velocities.
	 * \param[in] speed_vector name of the attribute with
	 *  speed vectors (dim 3 vector attribute)
	 */
	void compute_velocity(
	    const std::string& velocity = "v",
	    const std::string& speed_vector = "V"
	);

	/**
	 * \menu Post-processing
	 * \brief In a Euler simulation, compute the interface
	 *  between the two liquides.
	 * \param[in] domain the name of the mesh with the domain.
	 * \param[out] interface the name of the mesh where to store 
	 *  the interface.
	 * \param[in] primal if true, then interface is computed by
	 *  marching-tetrahedra from the primal 3d triangulation, else 
	 *  it is computed from the power diagram.
	 */
	void compute_interface(
	    const MeshGrobName& domain="omega",
	    const NewMeshGrobName& interface="interface",
	    const NewMeshGrobName& clip_region="",
	    bool primal=false
	);


	/**
	 * \menu Post-processing
	 * \brief Smoothes a surface by moving the points to the centroids
	 *  of their neighbors.
	 */
	void smooth_interface();

	/**
	 * \menu Post-processing
	 * \brief Computes the dual of a surface mesh.
	 * \param[in] dual the name of the created dual
	 *  surface mesh.
	 * \param[in] triangulate if true, triangulate the facets
	 */
	void compute_dual_surface_mesh(
	    const NewMeshGrobName& dual="dual",
	    bool triangulate=false
	);
	
	/**
	 * \menu Post-processing
	 * \brief Exports a mesh in raw format to be
	 *  used with Cyril Crassin's tools.
	 */
	void export_to_Cyril_Crassin_raw(
	    const std::string& basename="out"
	);


	/**
	 * \menu Post-processing
	 * \brief Extracts initial and final pointsets 
	 *  from an animated pointset.
	 */
	void extract_initial_and_final(
	    const NewMeshGrobName& now="EUR_now",	    
	    const NewMeshGrobName& initial="EUR_initial",
	    bool wrap_coords=false,
	    bool correct_origin=false,
	    index_t nb_per_axis=256
	);

	/**
	 * \menu Post-processing
	 * \brief Creates a scatter plot datafile for evaluating 
	 *  Early Universe Reconstruction quality.
	 */
	void EUR_scatter_plot(
	    const NewFileName& filename="scatter.dat",
	    index_t nb_per_axis = 256
	);

	
	/**
	 * \menu Post-processing
	 * \brief Selects a chart
	 */
	void select_chart(
	    const std::string& chart_attribute = "chart",
	    const std::string& selection = "selection",
	    index_t chart = 0
	);

	/**
	 * \menu Post-processing
	 * \brief Translates mesh by vector.
	 */
	void translate(double tx=0.0, double ty=0.0, double tz=0.0);
	
	/**
	 * \menu Post-processing
	 * \brief Set coordinates between 0 and 1 for reconstructed pointsets
	 *  with boundary conditions.
	 */
	void EUR_normalize_periodic_coordinates();

	/**
	 * \menu Post-processing
	 * \brief Copies a vertex attribute to geometry
	 */
	void copy_attribute_to_geometry(const std::string attribute);

	/**
	 * \menu Post-processing
	 * \brief Copies colors from the point with the same
	 *  index in another object
	 * \param[in] from the object from which colors
	 *  are copied
	 */
	void copy_point_colors(
	    const MeshGrobName& from
	);
	
	/**
	 * \menu Post-processing
	 * \brief Copies colors from the nearest point of 
	 *  another object
	 * \param[in] from the object from which colors
	 *  are copied
	 */
	void copy_nearest_point_colors(
	    const MeshGrobName& from
	);


	/**
	 * \menu Pre-processing
	 * \brief Samples the different regions of a volumetric
	 *  mesh with points.
	 * \param[in] points the name of the created pointset.
	 * \param[in] total_nb_points total number of points.
	 * \param region1_mass , region2_mass , region3_mass masses of
	 *  the points in the different regions. Leave 0 for regions with
	 *  no points.
	 */
	void sample_regions(
	    const std::string& points = "points",
	    index_t total_nb_points = 30000,
	    double region1_mass = 1e-3,
	    double region2_mass = 1.0,
	    double region3_mass = 0.0,
	    double region4_mass = 0.0,
	    double region5_mass = 0.0
	);

	/** 
	 * \menu Advection
	 * \brief Applies a velocity field to a mesh.
	 * \param[in] t0
	 */
	void advect(
	    velocity_field_t field,
	    double t0 = 0.0,
	    double dt = 0.01,
	    double nb_timesteps = 1,
	    time_integrator_t integrator = SIMPLE,
	    bool save_timesteps = false
	);

	/**
	 * \menu Misc
	 * \brief Save mesh normals to C format.
	 */
	void save_normals();

	/**
	 * \menu Misc
	 * \brief show hilbert curve.
	 */
	void show_Hilbert_curve(index_t dimension=2);

	/**
	 * \menu Misc
	 * \brief export point set and attribute in xyzw format
	 */
        void export_points_and_attribute(
	       const std::string& attribute_name,
               const NewFileName& file_name
	);

	/**
	 * \menu Post-processing
	 * \brief Resizes the warped mesh in such a way it has the same
	 *  volume as the initial condition.
	 */
	void normalize_transported_volume();

	/**
	 * \menu Post-processing
	 * \brief For tet6 files, extract trajectories
	 */
	void extract_trajectories();
        

        /**
	 * \menu Pre-processing
	 * \brief Inflates a mesh to encompass a given pointset
	 */
	void inflate(
	    const MeshGrobName& points="points",
	    double R0 = 0.0,
	    double R1 = 0.0,
	    index_t nb_rings = 0
	);
	
    };
}

#endif

