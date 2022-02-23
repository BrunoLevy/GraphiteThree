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
 

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_SELECTION_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_SELECTION_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_selection_commands.h
 * \brief Commands that manipulate selections in surface meshes.
 */

namespace OGF {

    /**
     * \brief Commands that manipulate surface meshes.
     */
    gom_class MESH_API MeshGrobSelectionCommands :
        public MeshGrobCommands {
            
    public:
            
       /**
        * \brief MeshGrobSurfaceCommands constructor.
        */
        MeshGrobSelectionCommands();

       /**
        * \brief MeshGrobSurfaceCommands destructor.
        */
        ~MeshGrobSelectionCommands() override;
	
    gom_slots:

        /**
         * \brief Selects all the vertices of the mesh.
         * \menu /Mesh/selection/vertices/
         */
        void select_all_vertices();

        /**
         * \brief Unselect all the vertices of the mesh.
         * \menu /Mesh/selection/vertices/
         */
        void unselect_all_vertices();

        /**
         * \brief Inverts vertices selection.
         * \details Makes selected vertices unselected and unselected
         *  vertices selected.
         * \menu /Mesh/selection/vertices/
         */
        void invert_vertices_selection();
        
        /**
         * \brief Selects all the vertices on the border of a surface.
         * \menu /Mesh/selection/vertices/
         */
        void select_vertices_on_surface_border();

        /**
         * \brief Unselects all the vertices on the border of a surface.
         * \menu /Mesh/selection/vertices/
         */
        void unselect_vertices_on_surface_border();

        /**
         * \brief Deletes the selected vertices
         * \menu /Mesh/selection/vertices/
	 * \param[in] remove_isolated if set, remove isolated vertices.
         */
        void delete_selected_vertices(bool remove_isolated=true);

	/**
	 * \brief Selects all the vertices that are duplicated in
	 *  a mesh.
	 * \param[in] tolerance maximum distance for considering
	 *  that two vertices are duplicated.
         * \menu /Mesh/selection/vertices/
	 */
	void select_duplicated_vertices(double tolerance=0.0);
    };
}

#endif


