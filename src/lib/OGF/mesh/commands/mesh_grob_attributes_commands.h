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
 *  (non-GPL) libraries:  Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_ATTRIBUTES_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_ATTRIBUTES_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_attributes_commands.h
 * \brief Commands that manipulate mesh attributes.
 */

namespace OGF {

    /**
     * \brief A class to select subsets in an array
     */
    class MESH_API Filter {
    public:
        /**
         * \brief Filter constructor
         * \details Throws an exception if description cannot
         *  be parsed
         * \param[in] size size of the array
         * \param[in] description description of the subset, as
         *  a ';'-separated list of selections:
         *   - '*' selects all elements
         *   - 'nnn' selects an individual element
         *   - 'nnn-mmm' selects the interval [nnn,mmm]
         *   - '!nnn' unselects an individual element
         *   - '!nnn-mmm' unselects the interval [nnn,mmm]
         *  Example: "*;!5" selects everything but element number 5
         * \param[in] floating_point if set, values to be tested are
         *  floating point values, else they are element indices.
         */
        Filter(
            index_t size, const std::string& description,
            bool floating_point=false
        );

        /**
         * \brief Tests an element
         * \param[in] item the element to be tested
         * \retval true if the element is in the subset
         * \retval false otherwise
         */
        bool test(index_t item) const;

        /**
         * \brief Tests an element by value
         * \param[in] item the element to be tested
         * \retval true if the element is in the subset
         * \retval false otherwise
         */
        bool test(double value) const;

    protected:
        /**
         * \brief used in 'items' mode (ctor, floating_point = false)
         */
        void parse_items(const std::string& destription);

        /**
         * \brief used in 'values' mode (ctor, floating_point = true)
         */
        void parse_values(const std::string& destription);        
        
    private:
        index_t size_;
        vector<double> include_items_;
        vector<std::pair<double, double> > include_intervals_;
        vector<double> exclude_items_;
        vector<std::pair<double, double> > exclude_intervals_;
    };
    
   /**
    * \brief Commands that manipulate mesh attributes.
    */
    gom_class MESH_API MeshGrobAttributesCommands : public MeshGrobCommands {
    public:

        /**
         * \brief MeshGrobAttributesCommands constructor.
         */
        MeshGrobAttributesCommands();

        /**
         * \brief MeshGrobAttributesCommands destructor.
         */
        ~MeshGrobAttributesCommands() override;
	
    gom_slots:


        /**
         * \brief Creates an attribute.
         * \param[in] name name of the attribute, without the localisation.
         * \param[in] where which mesh elements the attribute should 
         *  be attached to.
         * \param[in] type attribute type
         * \param[in] dimension number of components (1 for scalar)
         */
	gom_arg_attribute(where, handler, "combo_box")
	gom_arg_attribute(where, values, "vertices;edges;facets;cells")
	gom_arg_attribute(type, handler, "combo_box")
	gom_arg_attribute(type, values, "bool;uint32;int32;float64")
        void create_attribute(
            const std::string& name,
            const std::string& where = "points",
            const std::string& type  = "float64",
            index_t dimension = 1
        );
        
        /**
         * \brief Deletes an attribute.
         * \param[in] name the name of the attribute, 
         *   for instance "vertices.distance
         */
	gom_arg_attribute(name, handler, "combo_box")
	gom_arg_attribute(name, values, "$grob.attributes")
        void delete_attribute(const std::string& name);


        /**
         * \menu /Selection
         */
        void select_all();

        /**
         * \menu /Selection
         */
        void select_none();

        /**
         * \menu /Selection
         */
        void enlarge_selection(index_t nb_times=1);

        /**
         * \menu /Selection
         */
        void shrink_selection(index_t nb_times=1);


        /**
         * \menu /Selection
         */
        void close_small_holes_in_selection(index_t hole_size=1);
        
        /**
         * \menu /Selection
         */
        void invert_selection();

        /**
         * \menu /Selection
         */
        void delete_selected_elements(
            bool delete_isolated_vertices = true
        );
        
        /**
         * \menu /Selection
         */
        void hide_selection();
        
        /**
         * \menu /Selection/Vertices
         */
        void show_vertices_selection();

        /**
         * \brief Selects all the vertices on the border of a surface.
         * \menu /Selection/Vertices
         */
        void select_vertices_on_surface_border();

        /**
         * \brief Unselects all the vertices on the border of a surface.
         * \menu /Selection/Vertices
         */
        void unselect_vertices_on_surface_border();

	/**
	 * \brief Selects all the vertices that are duplicated in
	 *  a mesh.
	 * \param[in] tolerance maximum distance for considering
	 *  that two vertices are duplicated.
         * \menu /Selection/Vertices
	 */
	void select_duplicated_vertices(double tolerance=0.0);

        
        /**
         * \menu /Selection/Facets
         */
        void show_facets_selection();

        /**
         * \menu /Selection/Cells
         */
        void show_cells_selection();


        /**
         * \menu /Selection/Filters
         * \brief sets a filter
         * \param[in] where one of vertices, facets, cells
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
        gom_arg_attribute(where, handler, "combo_box")
        gom_arg_attribute(where, values, "vertices;facets;cells")
        void set_filter(
            const std::string& where, const std::string& filter="*",
            bool propagate=true
        );

        /**
         * \menu /Selection/Filters
         * \brief adds subsets to a filter
         * \param[in] where one of vertices, facets, cells
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
        gom_arg_attribute(where, handler, "combo_box")
        gom_arg_attribute(where, values, "vertices;facets;cells")
        void add_to_filter(
            const std::string& where, const std::string& filter,
            bool propagate=true
        );

        /**
         * \menu /Selection/Filters
         * \brief removes subsets from a filter
         * \param[in] where one of vertices, facets, cells
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
        gom_arg_attribute(where, handler, "combo_box")
        gom_arg_attribute(where, values, "vertices;facets;cells")
        void remove_from_filter(
            const std::string& where, const std::string& filter,
            bool propagate=true
        );

        /**
         * \menu /Selection/Filters
         * \brief propagates a filter from elements to all other
         *  elements (for instance, from cells to vertices and facets)
         * \param[in] from one of vertices, facets, cells
         */
        gom_arg_attribute(from, handler, "combo_box")
        gom_arg_attribute(from, values, "vertices;facets;cells")            
        void propagate_filter(const std::string& from);
        
        /**
         * \menu /Selection/Filters
         */
        gom_arg_attribute(where, handler, "combo_box")
        gom_arg_attribute(where, values, "vertices;facets;cells;all")
        void delete_filter(const std::string& where);
        
        /**
         * \brief Stores the vertices ids in an attribute.
         * \param[in] attribute the name of the vertex attribute
         * \menu Vertices
         */
        void compute_vertices_id(const std::string& attribute="id");

        
        /**
         * \brief Stores the edges ids in an attribute.
         * \param[in] attribute the name of the edge attribute
         * \menu Edges
         */
        void compute_edges_id(const std::string& attribute="id");

        /**
         * \brief Stores the facets ids in an attribute.
         * \param[in] attribute the name of the facet attribute
         * \menu Facets
         */
        void compute_facets_id(const std::string& attribute="id");

        /**
         * \brief Computes facets visibility from random views.
         * \param[in] nb_views number of views
	 * \param[in] dual_sided if true, facets seen as backfacing
	 *  count as negative.
         * \menu Facets
         */
        void compute_facets_visibility(
	    index_t nb_views = 1000,
	    bool dual_sided = true
	);
        
        /**
         * \brief Stores the cells ids in an attribute.
         * \param[in] attribute the name of the cell attribute
         * \menu Cells
         */
        void compute_cells_id(const std::string& attribute="id");
        
        /**
         * \brief Computes the distance between each vertex and a surface.
         * \param[in] surface the surface
         * \param[in] attribute the name of the vertex attribute
         * \menu Vertices
         */
        void compute_distance_to_surface(
            const MeshGrobName& surface,
            const std::string& attribute="distance"
        );

        /**
         * \brief Computes the distance to 
         *  an approximation of the medial axis of a surface.
         * \param[in] surface a pointset that samples the surface
         * \param[in] attribute the name of the vertex attribute
         * \menu Vertices
         */
        void compute_local_feature_size(
            const MeshGrobName& surface,            
            const std::string& attribute="lfs"
        );
	
	/**
	 * \brief Copies colors from a textured surface.
	 * \param[in] surface the surface mesh
	 * \param[in] texture the texture image file
	 * \param[in] copy_tex_coords if true, copy tex coords
	 *  in an additional attribute
	 */
	void copy_texture_colors(
            const MeshGrobName& surface,            	    
	    const ImageFileName& texture,
	    bool copy_tex_coords=false
	);

	
        /**
         * \brief Computes per-vertex ambient occlusion.
         * \param[in] attribute the name of the vertex attribute
	 * \param[in] nb_rays_per_vertex number of rays used to 
	 *  sample directions. The higher, the more precise.
	 * \param[in] nb_smoothing_iterations blur the result
	 *  a little bit to hide sampling noise
         * \menu Vertices
         */
        void compute_ambient_occlusion(
            const std::string& attribute="AO",
	    index_t nb_rays_per_vertex = 100,
	    index_t nb_smoothing_iterations = 2
        );

	
       /*********************************************************************/
        
    protected:
        void compute_sub_elements_id(
            MeshElementsFlags what, const std::string& attribute
        );

        /**
         * \brief Gets the selection displayed in the current shader, or
         *  MESH_NONE if no selection is displayed.
         */
        MeshElementsFlags visible_selection() const;
    };
    
}
#endif

