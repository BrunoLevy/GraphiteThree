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
 

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_SURFACE_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_SURFACE_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>
#include <geogram/parameterization/mesh_atlas_maker.h>
#include <geogram/parameterization/mesh_segmentation.h>

/**
 * \file OGF/mesh/commands/mesh_grob_surface_commands.h
 * \brief Commands that manipulate surface meshes.
 */

namespace OGF {

    /**
     * \brief Commands that manipulate surface meshes.
     */
    gom_class MESH_API MeshGrobSurfaceCommands :
        public MeshGrobCommands {
            
    public:
            
       /**
        * \brief MeshGrobSurfaceCommands constructor.
        */
        MeshGrobSurfaceCommands();

       /**
        * \brief MeshGrobSurfaceCommands destructor.
        */
        ~MeshGrobSurfaceCommands() override;
	
    gom_slots:

        /**
         * \menu Repair
         * \brief Tentatively fixes some defects in a surface
         * \param[in] epsilon Tolerance for merging vertices, 
         *  in % of bbox diagonal.
         * \param[in] min_comp_area Minimum area of a connected component, 
         *  in % of surface area
         * \param[in] max_hole_area Maximum area of holes to be filled, 
         *  in % of surface area.
         * \param[in] max_hole_edges Maximum number of edges 
         *  in each hole to be filled.
         * \param[in] max_degree3_dist Maximum distance 
         *  for removing degree 3 vertices, in % of bbox diagonal.
         * \param[in] remove_intersections Tentatively remove 
         *  self-intersections.
         */
        void repair_surface(
            double epsilon = 1e-6,
            double min_comp_area = 0.03,
            double max_hole_area = 1e-3,
            double max_hole_edges = 2000,
            double max_degree3_dist = 0.0,
            bool remove_intersections = false
        );

       /**********************************************************/

        /**
         * \menu Repair
         * \brief Merges vertices that are at the same location 
         *  or within tolerance
         * \param [in] epsilon Tolerance for merging vertices, 
         *  in % of bbox diagonal.
         */
        void merge_vertices(double epsilon = 1e-6);
        
       /**********************************************************/

        /**
         * \menu Repair
         * \brief Preprocessing for remesh_feature_sensitive
         * \param[in] margin margin to be added to borders, 
         *  in % of bbox diagonal.
         */
        void expand_border(double margin=0.05);

       /**********************************************************/

	/**
	 * \menu Repair
	 * \brief Fills the holes of the surface.
	 * \param[in] max_nb_vertices maximum number of vertices around
	 *   a hole.
	 */
	void fill_holes(index_t max_nb_vertices=0);


       /**********************************************************/	

        /**
         * \menu Repair
         * \brief Fixes facets orientation based on computed visibility.
         */
        void fix_facets_orientation();

       /**********************************************************/
        
        /**
         * \menu Repair
         * \brief Removes all facets that have visibility smaller than
         *  a given threshold.
         * \param[in] min_visibility minimum visibility to keep a facet
         */
        void remove_invisible_facets(double min_visibility = 0.005);
        
        
       /**********************************************************/
	
        /**
         * \menu Remesh
         * \brief Remeshes a (smooth) surface.
         * \param[in] remesh_name name of the generated surface mesh.
         * \param[in] nb_points desired number of points 
         *  in the generated mesh.
         * \param[in] tri_shape_adapt adapt triangle shapes 
         *  (0.0 means no shape adapation, 1.0 for moderate 
         *  shape adaptation, ...)
         * \param[in] tri_size_adapt adapt triangle sizes 
         *  (0.0 means no size adaptation, 1.0 for moderate 
         *  size adaptation, ...)            
	 * \param[in] adjust if true, adjust the triangles of
	 *  the remesh to better approximate the input mesh
	 * \param[in] adjust_max_edge_distance maximum adjustment,
	 *  relative to local average edge length 
         * \advanced
         * \param[in] normal_iter number of normal 
         *  smoothing iterations (if anisotropy is non-zero).
         * \param[in] Lloyd_iter number of Lloyd iterations for CVT.
         * \param[in] Newton_iter number of Newton iterations for CVT.
         * \param[in] Newton_m number of inner Newton iterations for CVT.
         * \param[in] LFS_samples number of samples. 
         *  used to compute gradation.
         */
        void remesh_smooth(
            const NewMeshGrobName& remesh_name = "remesh",
            unsigned int nb_points = 30000,
            double tri_shape_adapt = 1.0,
            double tri_size_adapt = 0.0,
	    bool adjust = true,
	    double adjust_max_edge_distance = 0.5,
            unsigned int normal_iter = 3,
            unsigned int Lloyd_iter = 5,
            unsigned int Newton_iter = 30,
            unsigned int Newton_m = 7,
            unsigned int LFS_samples = 10000
        );

       /**********************************************************/

        /**
         * \menu Remesh
         * \brief Remeshes a surface that has sharp features.
         * \param[in] surface_name name of the generated mesh.
         * \param[in] nb_points desired number of points 
         *  in the generated mesh.
         * \param[in] refine refine border in order to lower 
         *  Haussdorff distance.
         * \param[in] max_dist maximum distance to reference, 
         *  as a fraction of generated mesh average edge length.
         * \advanced
         * \param[in] normal_anisotropy determines how sharp features 
         *  are taken into account.
         * \param[in] nb_Lloyd_iter number of Lloyd iterations for CVT.
         * \param[in] nb_Newton_iter number of Newton iterations for CVT.
         * \param[in] nb_LpCVT_iter number of feature-sensitive iterations.
         * \param[in] Newton_m number of inner Newton iterations for CVT.
         * \param[in] RVC_centroids use centroids of restricted Voronoi cells.
         */
        void remesh_feature_sensitive(
            const NewMeshGrobName& surface_name = "remesh",
            unsigned int nb_points = 30000,
            bool refine = false,
            double max_dist = 0.5,
            double normal_anisotropy = 5.0,
            unsigned int nb_Lloyd_iter = 5,
            unsigned int nb_Newton_iter = 30,
            unsigned int nb_LpCVT_iter = 10,
            unsigned int Newton_m = 7,
            bool RVC_centroids = false
        );
        
       /**********************************************************/

	/**
	 * \menu Remesh
	 * \brief Generates a quad-dominant mesh
         * \param[in] surface_name name of the generated mesh.
	 * \param[in] rel_edge_len relative edge length.
	 * \param[in] sharp_features true for CAD mesh, false otherwise
	 * \param[in] optimize_parity tentatively optimize parity.
	 * \param[in] max_scaling_corr maximum scaling correction 
	 *  (use 1.0 to disable).
	 */
	void remesh_quad_dominant(
            const NewMeshGrobName& surface_name = "remesh",
	    double rel_edge_len = 1.0,
	    bool sharp_features = true,
	    bool optimize_parity = false,
	    double max_scaling_corr = 1.0
	);

       /**********************************************************/

        /**
	  * \menu Boolean operations
	  * \brief Computes the union between two meshes.
          * \param[in] other name of the other mesh
          * \param[in] result name of the result mesh
	  * \param[in] pre_process triangulate, inputs, remove small edges, 
	  *  make sure there is no intersection
	  * \param[in] post_process triangulate result, remove small edges, 
	  *  make sure there is no intersection
          */
        void compute_union(
	    const MeshGrobName& other,
	    const NewMeshGrobName& result = "result",
	    bool pre_process=false,	    
	    bool post_process=false
        );


        /**
	  * \menu Boolean operations
	  * \brief Computes the intersection between two meshes.
          * \param[in] other name of the other mesh
          * \param[in] result name of the result mesh
	  * \param[in] pre_process triangulate, inputs, remove small edges, 
	  *  make sure there is no intersection
	  * \param[in] post_process triangulate result, remove small edges, 
	  *  make sure there is no intersection
          */
        void compute_intersection(
	    const MeshGrobName& other,
	    const NewMeshGrobName& result = "result",
	    bool pre_process=false,
	    bool post_process=false	    
        );

        /**
	  * \menu Boolean operations
	  * \brief Computes the difference between two meshes.
          * \param[in] other name of the other mesh
          * \param[in] result name of the result mesh
	  * \param[in] pre_process triangulate, inputs, remove small edges, 
	  *  make sure there is no intersection
	  * \param[in] post_process triangulate result, remove small edges, 
	  *  make sure there is no intersection
          */
        void compute_difference(
	    const MeshGrobName& other,
	    const NewMeshGrobName& result = "result",
	    bool pre_process=false,
	    bool post_process=false	    
        );


       /**********************************************************/
	
       /**
        * \menu Remesh
        * \brief Simplifies a surface using vertex clustering.
        * \param[in] nb_bins the higher, the more detailed mesh.
        * \param[in] remove_deg3_vrtx if true, remove degree 3 vertices
        * \param[in] keep_borders if true, do not decimate vertices 
        *  on the border
        * \param[in] repair if true, repairs the mesh to remove non-manifold
        *  edges and borders
        */
       void decimate(
           index_t nb_bins = 100,
           bool remove_deg3_vrtx = true,
           bool keep_borders = true,
           bool repair = true
       );

       /**********************************************************/

	/**
         * \menu Subdivision
	 * \brief Splits all the triangles of a simplicial mesh into 
	 *  four triangles.
	 * \param[in] nb_times number of times the triangles are split.
	 */
	void split_triangles(index_t nb_times=1);

	/**
         * \menu Subdivision
	 * \brief Splits all the facets of a mesh into quads.
	 * \param[in] nb_times number of times the facets are split.
	 */
	void split_quads(index_t nb_times=1);


	/**
         * \menu Subdivision
	 * \brief Splits all the facets of a mesh into quads 
	 *  using Catmull-Clark subdivision.
	 * \param[in] nb_times number of times the facets are split.
	 */
	void split_catmull_clark(index_t nb_times=1);
	
	/**
	 * \menu Subdivision
	 * \brief Subdivides the facets until they have less than
	 *  the specified number of vertices.
	 */
	void tessellate_facets(index_t max_vertices_per_facet=4);


	/**
	 * \menu Subdivision
	 * \brief Triangulates all the facets.
	 */
	void triangulate() {
	    tessellate_facets(3);
	}
	

	/**
	 * \menu Subdivision
	 * \brief Triangulates all the facets by inserting
	 *   a vertex in the center of each facet.
	 */
	void triangulate_center_vertex();

	   
	/**
	 * \brief Smooths the mesh by optimizing the vertices that
	 *   are not selected. Selected vertices are locked.
	 */
	void smooth();

       /**********************************************************/

	enum Parameterizer {
	    LSCM, SpectralLSCM, ABFplusplus
	};

        /**
         * \menu Atlas/Segmentation
         * \brief Segments a mesh.
         */
        void segment(
            MeshSegmenter segmenter=SEGMENT_GEOMETRIC_VSA_L2,
            index_t nb_segments=10
        );
        
	/**
	 * \menu Atlas/Segmentation
	 * \brief Gets the charts attribute from a parameterized mesh.
	 */
	void get_charts();

	/**
	 * \menu Atlas/Segmentation
	 * \brief Removes the charts attribute.
	 */
        void remove_charts();
        
	/**
	 * \menu Atlas/Segmentation
	 * \brief Unglues facet edges based on specified angle
	 * \param[in] angle_threshold unglue facets along edge 
	 *  if angle between adjacent facet is larger than threshold
	 */
	void unglue_sharp_edges(double angle_threshold=90);

	/**
	 * \menu Atlas/Segmentation
	 * \brief Unglues facet edges adjacent to two different charts
	 */
	void unglue_charts();
        
	/**
	 * \menu Atlas/Parameterization
	 * \brief Computes texture coordinates of a surface.
	 * \param[in] detect_sharp_edges if true, generate chart boundary
         *  on edges with angle is larger than threshold.
	 * \param[in] sharp_edges_threshold if the angle between the normals
	 *  of two adjacent facets is larger than this threshold then the edge
	 *  will be a chart boundary.
	 * \param[in] param the algorithm used for parameterizing the charts.
	 *  use ABF for best quality, or LSCM for faster result.
	 * \param[in] verbose if true, display statistics during atlas
	 *  generation.
	 * \param[in] pack one of PACK_TETRIS, PACK_XATLAS
	 */
	void make_texture_atlas(
	    bool detect_sharp_edges = false,
	    double sharp_edges_threshold = 45.0,
	    ChartParameterizer param=PARAM_ABF,
	    ChartPacker pack=PACK_XATLAS,
	    bool verbose=false
	);


	/**
	 * \menu Atlas/Parameterization
	 * \brief Packs charts in texture space
	 * \param[in] pack one of PACK_TETRIS, PACK_XATLAS
	 */
	void pack_texture_space(
	    ChartPacker pack=PACK_XATLAS
	);
	
	/**
	 * \menu Atlas/Parameterization
	 * \brief Computes texture coordinates of a single unfoldable surface.
	 * \param[in] attribute the name of the attribute that will store
	 *  texture coordinates.
	 * \param[in] param one of LSCM, SpectralLSCM, ABFplusplus.
	 * \param[in] verbose if true, display statistics during atlas
	 *  generation.
	 */
	void parameterize_chart(
	    const std::string& attribute="tex_coord",
	    ChartParameterizer param=PARAM_LSCM,
	    bool verbose=false
	);

	/**
	 * \menu Atlas/Baking
	 * \brief Bakes normals from a surface to the texture atlas.
	 * \param[in] surface the name of the mesh with normals to be baked.
	 *  can be the current mesh or another one with higher-resolution
	 *  normals.
	 * \param[in] size pixel-size of the generated normal map.
	 * \param[in] image filename of the generated normal map.
	 * \param[in] nb_dilate number of dilations 
	 * \param[in] tex_coord the name of the facet corner attribute that 
	 *  stores texture coordinates.
	 */
	void bake_normals(
	    const MeshGrobName& surface,
	    index_t size=1024,
	    const NewImageFileName& image="normals.png",
	    index_t nb_dilate=2,
	    const std::string& tex_coord="tex_coord"
	);


	/**
	 * \menu Atlas/Baking
	 * \brief Bakes colors from a surface to the texture atlas.
	 * \param[in] surface the name of the mesh with colors to be baked.
	 *  can be the current mesh or another one with higher-resolution.
	 * \param[in] color the name of the attribute with the colors to be
	 *  baked.
	 * \param[in] size pixel-size of the generated normal map.
	 * \param[in] image filename of the generated normal map.
	 * \param[in] nb_dilate number of dilations 
	 * \param[in] attribute the name of the facet corner attribute that 
	 *  stores texture coordinates.
	 */
	void bake_colors(
	    const MeshGrobName& surface,
	    const std::string& color="color",
	    index_t size=1024,
	    const NewImageFileName& image="colors.png",
	    index_t nb_dilate=2,
	    const std::string& attribute="tex_coord"
	);


	/**
	 * \menu Atlas/Baking
	 * \brief Bakes texture from a textured surface to an atlas.
	 * \param[in] src_surface the name of the mesh with the 
	 *   texture to be baked.
	 *  can be the current mesh or another one with higher-resolution
	 *  normals
	 * \param[in] src_texture the texture associated with src_surface
	 * \param[in] src_tex_coord the source facet corner attribute 
	 *            with the texture coordinates
	 * \param[in] size pixel-size of the generated texture
	 * \param[in] image filename of the generated texture
	 * \param[in] nb_dilate number of dilations 
	 * \param[in] tex_coord the name of the facet corner attribute that 
	 *  stores texture coordinates for the generated texture
	 */
	void bake_texture(
	    const MeshGrobName& src_surface,
	    const ImageFileName& src_texture,
	    const std::string& src_tex_coord="tex_coord",
	    index_t size=1024,
	    const NewImageFileName& image="texture.png",
	    index_t nb_dilate=2,
	    const std::string& tex_coord="tex_coord"
	);

	
    };
}
#endif

