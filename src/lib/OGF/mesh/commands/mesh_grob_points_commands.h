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
 * As an exception to the GPL, Graphite can be linked 
 *  with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_POINTS_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_POINTS_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_points_commands.h
 * \brief Commands that manipulate point sets.
 */
namespace OGF {

    /**
     * \brief Commands that manipulate point sets.
     */
    gom_class MESH_API MeshGrobPointsCommands : public MeshGrobCommands {
    public:
        /**
         * \brief MeshGrobPointsCommands constructor.
         */
        MeshGrobPointsCommands() ;

        /**
         * \brief MeshGrobPointsCommands destructor.
         */
        ~MeshGrobPointsCommands() override;
	
    gom_slots:

        /********************************************************/
	
        /**
	 * \menu Preprocessing
         * \brief Smoothes a pointset by projection onto local planes.
         * \param[in] nb_iterations number of smoothing iterations.
         * \param[in] nb_neighbors number of neighbors for estimating 
         *   tangent plane. 
         */
        void smooth_point_set(
            unsigned int nb_iterations = 1,
            unsigned int nb_neighbors = 30
        );

        /********************************************************/
	
	/**
	 * \menu Preprocessing
	 * \brief Marks isolated points as selection.
	 * \param[in] nb number of points in neighborhood
	 * \param[in] radius maximum neighborhood size
	 * \param[in] relative_radius radius is relative to 
	 *   object bbox diagonal.
	 */
        void detect_outliers(
	   index_t nb = 10,
	   double radius = 0.01,
	   bool relative_radius = true
	);

        /********************************************************/
	
	/**
	 * \menu Preprocessing
	 * \brief Estimates the normal vector to a point-sampled surface
	 *  using K nearest neighbors. The computed normals are stored
	 *  in the "normal" attribute.
	 * \details Only normal directions are estimated. Normal orientations
	 *  may be incoherent. This may require an additional bread-first
	 *  traversal of the KNN graph to coherently orient normals.
	 * \param[in] nb_neighbors number of nearest neighbors (K).
	 * \param[in] reorient if true, try to enforce coherent normal
	 *  orientations by propagation over the KNN graph.
	 * \retval true if normals were successfully estimated.
	 * \retbval false otherwise (when the user pushes the cancel button).
	 */
	bool estimate_normals(index_t nb_neighbors = 30, bool reorient=true);
	
        /********************************************************/	

	/**
	 * \menu Preprocessing
	 * \brief Estimates the density.
	 * \param[in] radius estimated density is 
	 *   one over number of points within radius
	 * \param[in] relative_radius radius is relative to 
	 *   object bbox diagonal.
	 * \param[in] attribute name of the attribute
	 *   where to store the estimated density
	 */
	void estimate_density(
	    double radius = 0.005,
	    bool relative_radius = true,
	    const std::string& attribute = "density"
	);

        /********************************************************/	
	
        /**
	 * \menu Reconstruction
	 * \brief Reconstructs a surface from a point set using
	 *  Simple and Scalable Surface Reconstruction.
	 * \param[in] radius search radius for neighborhoods 
	 *  (in % of bbox diagonal)
	 * \advanced
	 * \param[in] nb_smoothing_iterations number of smoothing iterations
	 * \param[in] nb_neighbors number of neighbors 
	 *  for estimating tangent plane
	 */
        void reconstruct_surface_SSSR(
            double radius = 5.0,
            unsigned int nb_smoothing_iterations = 1,
            unsigned int nb_neighbors = 30
        );

        /********************************************************/

        /**
	 * \menu Reconstruction
         * \brief Reconstructs a surface from points and normals using
         *  Misha Kahzdan's Screened Poisson Reconstruction.
         * \param[in] reconstruction the name of the reconstructed surface
         * \param[in] depth the depth of the octree, 8 is the default value,
         *  use 10 or 11 for highly detailed models
         */
        void reconstruct_surface_Poisson(
            const NewMeshGrobName& reconstruction = "reconstruction",
            unsigned int depth = 8
        );
        
        /********************************************************/

	/**
	 * \menu Reconstruction
	 * \brief Reconstructs a surface from points using a 2D Delaunay
	 *  triangulation. Can be used for Digital Elevation Models.
	 */
	void reconstruct_surface_Delaunay2d();

        /********************************************************/
	

        /**
	 * \menu Sampling
         * \brief Creates a pointset that samples a surface.
         * \param[in] points the created pointset
         * \param[in] nb_points number of points
         * \param[in] copy_normals if set, normals are copied from
         *  the surface
         * \advanced
         * \param[in] Lloyd_iter number of Lloyd iterations for CVT.
         * \param[in] Newton_iter number of Newton iterations for CVT.
         * \param[in] Newton_m number of inner Newton iterations for CVT.
         */
        void sample_surface(
            const NewMeshGrobName& points = "points",
            bool copy_normals = false,
            unsigned int nb_points = 30000,
            unsigned int Lloyd_iter = 5,
            unsigned int Newton_iter = 30,
            unsigned int Newton_m = 7
        );

        /********************************************************/

        /**
	 * \menu Sampling
         * \brief Creates a pointset that samples a volume.
         * \param[in] points the created pointset
         * \param[in] nb_points number of points
         * \advanced
         * \param[in] Lloyd_iter number of Lloyd iterations for CVT.
         * \param[in] Newton_iter number of Newton iterations for CVT.
         * \param[in] Newton_m number of inner Newton iterations for CVT.
         */
	void sample_volume(
            const NewMeshGrobName& points = "points",
            unsigned int nb_points = 30000,
            unsigned int Lloyd_iter = 5,
            unsigned int Newton_iter = 30,
            unsigned int Newton_m = 7
	);
	

        /********************************************************/

	/**
	 * \brief Delete all points marked as selection.
	 */
	void delete_selected_points();

        /********************************************************/
        
        /**
         * \brief Creates a new vertex at given coordinates.
         * \param[in] x , y , z the coordinates of the point
	 * \param[in] selected if true, mark created vertex as selection
         */
        void create_vertex(
            double x,
            double y,
            double z,
	    bool selected = false
        );

	
    };
        
    /********************************************************/
    
} 
    
#endif

