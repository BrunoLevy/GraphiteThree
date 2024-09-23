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


#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_SHAPES_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_SHAPES_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_shapes_commands.h
 * \brief Commands that create simple shapes.
 */

namespace OGF {

    /**
     * \brief Commands that create simple shapes.
     */
    gom_class MeshGrobShapesCommands : public MeshGrobCommands {
    public:

        /**
         * \brief MeshGrobShapesCommands constructor.
         */
        MeshGrobShapesCommands();

        /**
         * \brief MeshGrobShapesCommands destructor.
         */
        ~MeshGrobShapesCommands() override;

    gom_slots:

        /**
         * \brief Creates a new square in the current mesh.
         * \menu /Surface/Shapes
         */
        void create_quad(
	    const vec3& p1 = vec3(0,0,0),
	    const vec3& p2 = vec3(1,0,0),
	    const vec3& p3 = vec3(1,1,0),
	    const vec3& p4 = vec3(0,1,0)
        );

        /**
         * \brief Creates a new cube in the current mesh.
         * \menu /Surface/Shapes
         */
        void create_box(
	    const vec3& pmin = vec3(0,0,0),
	    const vec3& pmax = vec3(1,1,1)
        );

        /**
         * \brief Creates a new cylinder in the current mesh.
         * \menu /Surface/Shapes
         */
        void create_cylinder_from_axes(
            const vec3& center  = vec3(0,0,0),
            const vec3& X_axis  = vec3(0.25,0,0),
            const vec3& Y_axis  = vec3(0,0.25,0),
            const vec3& Z_axis  = vec3(0,0,1),
            index_t precision = 10,
	    bool capping = true
        );

        /**
         * \brief Creates a new cylinder in the current mesh.
         * \menu /Surface/Shapes
         */
	void create_cylinder_from_extremities(
	    const vec3& p1 = vec3(0,0,-0.5),
	    const vec3& p2 = vec3(0,0, 0.5),
	    double radius = 1,
	    index_t precision = 10,
	    bool capping = true
	);

        /**
         * \brief Creates a new icosahedron in the current mesh.
         * \menu /Surface/Shapes
         */
        void create_icosahedron(
	    const vec3& center = vec3(0,0,0),
	    double radius = 1.0
	);

	/**
	 * \brief Creates a mesh that approximates a sphere.
	 * \param[in] precision number of time the initial icosahedron is split
	 *  (there are 20*4^precision triangles in the final mesh)
	 * \menu /Surface/Shapes
	 */
	void create_sphere(
	    const vec3& center = vec3(0,0,0),
	    double radius = 1.0,
	    index_t precision = 4
	);

        /**
         * \brief Creates a new ngon in the current mesh.
         * \menu /Surface/Shapes
         */
        void create_ngon(
            const vec3& center  = vec3(0,0,0),
	    double R = 1.0,
            index_t nb_edges = 6,
	    bool triangulate = true
        );

	/**
	 * \brief Creates a surfacic mesh from the bounding box of an object.
	 * \param[in] grob the object
	 * \param[in] nb_split the number of times the facets of the bounding
	 *  box should be split into smaller quads.
         * \menu /Surface/Shapes
	 */
	void create_from_bounding_box(
	    const GrobName& grob,
	    index_t nb_split = 0
	);
    };

}

#endif
