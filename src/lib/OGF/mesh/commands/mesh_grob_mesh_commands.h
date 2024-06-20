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
 *  (non-GPL) libraries: Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_MESH_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_MESH_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_mesh_commands.h
 * \brief Commands that manipulate the mesh in a MeshGrob
 */
namespace OGF {

    /**
     * \brief Commands that manipulate the mesh in a MeshGrob
     */
    gom_class MESH_API MeshGrobMeshCommands : public MeshGrobCommands {
    public:

        /**
         * \brief MeshGrobMeshCommands constructor.
         */
        MeshGrobMeshCommands() ;

        /**
         * \brief MeshGrobMeshCommands destructor.
         */
        ~MeshGrobMeshCommands() override;
        
    gom_slots:
        
        /**
         * \brief displays some statistics about the current mesh.
         */
        void display_statistics();

        /**
         * \brief computes and displays some topological invariants.
         */
        void display_topology();

        /**
         * \brief Copies a mesh.
         * \param[in] name name of the newly created mesh.
         * \param[in] edges copy edges.
         * \param[in] facets copy facets.
         * \param[in] cells copy cells.
         * \param[in] attributes copy attributes.
         * \param[in] kill_isolated_vx if true, vertices 
         *  that are no longer connected to anything are discarded.
         */
        void copy(
            const std::string& name,
            bool edges = true,
            bool facets = true,
            bool cells = true,
            bool attributes = true,
            bool kill_isolated_vx = false
        );
        
        /**
         * \brief Remove mesh elements.
         * \param[in] vertices if set, removes everything !
         * \param[in] edges remove all mesh edges.
         * \param[in] facets remove all mesh facets.
         * \param[in] cells remove all mesh cells.
         * \param[in] kill_isolated_vx if true, vertices.
         *  that are no longer connected to anything are discarded.
         */
        void remove_mesh_elements(
            bool vertices=false,
            bool edges=false,
            bool facets=false,
            bool cells=false,
            bool kill_isolated_vx=false
        );

        /**
         * \brief Remove isolated vertices.
         */
        void remove_isolated_vertices();

        /**
         * \brief Scales and translates a mesh to fit within a sphere.
         * \param[in] Cx x coordinate of the center.
         * \param[in] Cy y coordinate of the center.
         * \param[in] Cz z coordinate of the center.
         * \param[in] radius radius of the bounding sphere.
         */
        void normalize_mesh(
            double Cx = 0.0,
            double Cy = 0.0,
            double Cz = 0.0,
            double radius = 1.0
        );

	/**
	 * \brief Scales and translates a mesh to fit within a box.
	 * \param xmin , ymin , zmin, xmax , ymax, zmax extent of the
	 *  box.
	 * \param[in] uniform if true, coordinates are scaled uniformly,
	 *  else they are scaled to fit the box exactly.
	 */
	void normalize_mesh_box(
	    double xmin = 0.0,
	    double ymin = 0.0,
	    double zmin = 0.0,
	    double xmax = 1.0,
	    double ymax = 1.0,
	    double zmax = 1.0,
	    bool uniform = true
	);


        /**
         * \brief Appends a surface mesh to this mesh. Note: for now,
         *  only works for surfaces, and does not append attributes
         *  (TODO...). It merges the duplicated vertices and facets.
         */
        void append(const MeshGrobName& other);

        /**
         * \brief Gathers all surface meshes into a single surface
         *  mesh.
         */
        void gather(const NewMeshGrobName& new_mesh);
    };
}

#endif

