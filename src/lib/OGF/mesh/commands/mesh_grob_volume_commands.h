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
 * As an exception to the GPL, Graphite can be linked with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_VOLUME_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_VOLUME_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_volume_commands.h
 * \brief Commands that manipulate volume meshes.
 */

namespace OGF {

   /**
    * \brief Commands that manipulate volume meshes.
    */
    gom_class MESH_API MeshGrobVolumeCommands : public MeshGrobCommands {
    public:

        /**
         * \brief MeshGrobVolumeCommands constructor.
         */
        MeshGrobVolumeCommands();

	/**
	 * \brief MeshGrobVolumeCommands destructor.
	 */
	 ~MeshGrobVolumeCommands() override;
	
    gom_slots:

       /*********************************************************************/
        
        /**
         * \brief Fills a closed surface mesh with tetrahedra, using tetgen.
         * \param[in] preprocess Tentatively fix degeneracies 
         *  in the input mesh
         * \param[in] refine Create additional vertices to improve quality.
         * \param[in] quality 1.0 for high quality, 5.0 for low quality.
         * \param[in] verbose enables tetgen statistics and messages.
         */
        void tet_meshing(
            bool preprocess=true,
            bool refine=true,
            double quality=1.0,
            bool verbose=false,
	    bool keep_regions=false
        );

       /*********************************************************************/

        /**
         * \brief Fills a closed mesh with hexaedra (+other cells).
         * \param[in] hexdom_name name of the generated mesh.
         * \param[in] nb_points desired number of points
         *  in the generated mesh.
         * \param[in] prisms generate prisms.
         * \param[in] pyramids generate pyramids.
         * \param[in] border_refine refine border in order to lower 
         *  Haussdorff distance.
         * \param[in] border_max_dist maximum distance to reference, 
         *  as a fraction of input mesh average edge length (TODO: check).
         * \advanced
         * \param[in] min_normal_cos minimum angle cosine 
         *  between two triangular faces merged into an hex face.
         * \param[in] max_corner_cos  maximum angle cosine 
         *  at a quad corner.
         */
        void hex_dominant_meshing(
            const NewMeshGrobName& hexdom_name = "hexes",
            unsigned int nb_points = 30000,
            bool prisms = true,
            bool pyramids = true,
            bool border_refine = false,
            double border_max_dist = 0.2,
            double min_normal_cos = 0.5,
            double max_corner_cos = 0.6
        );


        /*********************************************************************/

	enum VoronoiSimplification {
	    keep_everything,
	    simplify_tet,
	    simplify_tet_voro
	};

	/**
	 * \brief Generates a Voronoi mesh from a volume and points.
	 * \param[out] voronoi name of the generated Voronoi mesh
	 * \param[in] nb_cells number of Voronoi cells to generate
	 * \param[in] simplification specifies how cells and faces should
	 *  be simplified
	 * \param[in] angle_threshold (in degrees) simplify boundary edges 
	 *  whenever their facets have normal angle smaller than threshold
	 * \param[in] shrink optional shrink factor for displaying cells
	 * \advanced
	 * \param[in] points optional name of the pointset used to compute the
	 *  Voronoi diagram
	 * \param[in] exact if true, all intersection perdicates are 
	 *  evaluated with exact arithmetics and symbolic perturbations
	 * \param[in] generate_ids if true, generate ids in attributes. 
	 *  Necessary to output OVM files.
	 * \param[in] medial_axis if true, generate an approximation of 
	 *  the medial axis (by removing facets from the computed mesh).
	 */
	void Voronoi_meshing(
	    const NewMeshGrobName& voronoi = "voronoi",
	    index_t nb_cells = 1000,
	    VoronoiSimplification simplification = simplify_tet_voro,
	    double angle_threshold = 1e-3,
	    double shrink = 0.0,
	    const NewMeshGrobName& points = "",
	    bool exact = true,
	    bool tessellate_non_convex = false,
	    bool generate_ids = true,
	    bool medial_axis = false
	);
	

        /*********************************************************************/
	
        /**
         * \brief Computes and displays various statistics for a hex-dominant
         *  mesh.
         * \param[in] save_histo if true, save dihedral and facet angle
         *  histograms
         * \param[in] nb_bins number of bins in the computed histograms
         */
        void volume_mesh_statistics(
            bool save_histo=false,            
            index_t nb_bins=100
        );
        
        /*********************************************************************/

        /**
         * \brief Creates a tetrahedral mesh from a closed surface mesh 
         *  and a pointset, using tetgen. Initial closed surface is remeshed.
         * \param[in] points a pointset that will be inserted into the 
         *  generated tetrahedral mesh
         * \param[in] tetrahedra the name of the generated tetrahedralized
         *  mesh
         * \param[in] refine_surface if true, insert vertices until distance to
         *  original surface is smaller than \p max_distance
         * \param[in] max_distance maximum distance to original surface,
         *  as a proportion of average edge length of original surface
         * \menu Advanced
         */
        void tet_meshing_with_points(
            const MeshGrobName& points,
            const NewMeshGrobName& tetrahedra = "tetrahedra",
            bool refine_surface = false,
            double max_distance = 0.2
        );

        /*********************************************************************/

        /**
         * \brief Converts a tetrahedral mesh into a hexahedral dominant
         *  mesh by merging tetrahedra
         * \param[in] hexdom_name name of the generated mesh.
         * \param[in] prisms generate prisms.
         * \param[in] pyramids generate pyramids.
         * \advanced
         * \param[in] min_normal_cos minimum angle cosine 
         *  between two triangular faces merged into an hex face.
         * \param[in] max_corner_cos  maximum angle cosine 
         *  at a quad corner.
         * \menu Advanced
         */
        void tet2hex(
            const NewMeshGrobName& hexdom_name = "hexes",
            bool prisms = true,
            bool pyramids = true,
            double min_normal_cos = 0.5,
            double max_corner_cos = 0.6
        );

        /*********************************************************************/

        /**
	 * \brief Extracts the boundary of the zone with tetrahedral cells and 
	 *  remeshes it using Tetgen.
         * \param[in] quality 1.0 for high quality, 5.0 for low quality.
         * \menu Advanced
         */
        void remesh_tetrahedra(double quality = 0.5);

        /*********************************************************************/

	/**
	 * \brief Displays the volume of a mesh.
	 */
	void display_volume();

        /*********************************************************************/

	/**
	 * \brief Copies the border of the volume into the surfacic part
	 *  of the mesh.
	 */
	void compute_borders();
    };
    
}
#endif

