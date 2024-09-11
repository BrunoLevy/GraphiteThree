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


#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_SELECTIONS_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_SELECTIONS_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_selections_commands.h
 * \brief Commands that manipulate mesh selections.
 */

namespace OGF {


   /**
    * \brief Commands that manipulate mesh selections.
    */
    gom_class MESH_API MeshGrobSelectionsCommands : public MeshGrobCommands {
    public:

        /**
         * \brief MeshGrobSelectionsCommands constructor.
         */
        MeshGrobSelectionsCommands();

        /**
         * \brief MeshGrobSelectionsCommands destructor.
         */
        ~MeshGrobSelectionsCommands() override;

    gom_slots:

        void select_all();

        void select_none();

        void enlarge_selection(index_t nb_times=1);

        void shrink_selection(index_t nb_times=1);

        void close_small_holes_in_selection(index_t hole_size=1);

        void invert_selection();

        void delete_selected_elements(
            bool delete_isolated_vertices = true
        );

        void hide_selection();


        /**
         * \brief sets the visible selection
         * \param[in] selection semi-column-separated list of
         *  star,id,id1-id2,!id,!id1-id2
         */
        void set_selection(const std::string& selection="*");

        /**
         * \menu Vertices
         */
        void show_vertices_selection();

        /**
         * \brief Selects all the vertices on the border of a surface.
         * \menu Vertices
         */
        void select_vertices_on_surface_border();

        /**
         * \brief Unselects all the vertices on the border of a surface.
         * \menu Vertices
         */
        void unselect_vertices_on_surface_border();

	/**
	 * \brief Selects all the vertices that are duplicated in
	 *  a mesh.
	 * \param[in] tolerance maximum distance for considering
	 *  that two vertices are duplicated.
         * \menu Vertices
	 */
	void select_duplicated_vertices(double tolerance=0.0);

        /**
         * \brief Selects all the vertices on triangles with
         *  their three vertices that are colinear.
         * \menu Vertices
         */
        void select_vertices_on_degenerate_facets();

        /**
         * \menu Facets
         */
        void show_facets_selection();


        /**
         * \brief Selects facets with zero-area
         * \param[in] add_to_selection if set, do not clear selection
         * \menu Facets
         */
        void select_degenerate_facets(bool add_to_selection=false);

        /**
         * \brief Selects facets that have an intersection with another
         *  facet of the same mesh.
         * \param[in] add_to_selection if set, do not clear selection
         * \param test_adjacent_facets test also pairs of facets
         *  that share an edge or a vertex
         * \menu Facets
         */
        void select_intersecting_facets(
            bool add_to_selection=false, bool test_adjacent_facets=true
        );

        /**
         * \brief For each set of duplicated facet, select one facet of
         *  the set.
         * \menu Facets
         */
        void select_duplicated_facets();

        /**
         * \brief Selects the facets incident to the border
         */
        void select_facets_on_border();

        /**
         * \brief Selects facets that have all their vertices selected
         * \menu Facets
         */
        void select_facets_from_vertices_selection();

        /**
         * \menu Cells
         */
        void show_cells_selection();

    protected:
        /**
         * \brief Gets the selection displayed in the current shader, or
         *  MESH_NONE if no selection is displayed.
         */
        MeshElementsFlags visible_selection() const;
    };
}

#endif
