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


#ifndef H_OGF_VOXEL_COMMANDS_VOXEL_GROB_ATTRIBUTES_COMMANDS_H
#define H_OGF_VOXEL_COMMANDS_VOXEL_GROB_ATTRIBUTES_COMMANDS_H

#include <OGF/voxel/common/common.h>
#include <OGF/voxel/commands/voxel_grob_commands.h>
#include <OGF/mesh/grob/mesh_grob.h>

/**
 * \file OGF/voxel/commands/voxel_grob_attributes_commands.h
 * \brief Commands that manipulate voxel attributes.
 */

namespace OGF {

   /**
    * \brief Commands that manipulate voxel attributes.
    */
    gom_class VOXEL_API VoxelGrobAttributesCommands : public VoxelGrobCommands {
    public:

        /**
         * \brief VoxelGrobAttributesCommands constructor.
         */
        VoxelGrobAttributesCommands();

        /**
         * \brief VoxelGrobAttributesCommands destructor.
         */
        ~VoxelGrobAttributesCommands() override;

    gom_slots:

        /**
         * \brief Initializes a voxel grob from an object box.
         * \param[in] object name of the object to copy the bounding box from, or
         *  empty screen for unit box.
         * \param[in] nu number of cells along the U axis
         * \param[in] nv number of cells along the V axis
         * \param[in] nw number of cells along the W axis
         * \menu /Geometry
         */
        void init_box_from_object(
            const GrobName& object,
            index_t nu = 128,
            index_t nv = 128,
            index_t nw = 128
        );

        /**
         * \brief Deletes an attribute.
         * \param[in] name the name of the attribute
         */
        void delete_attribute(const std::string& name);

        /**
         * \brief Normalizes a floating-point attribute.
         * \param[in] name the name of the attribute
         * \param[in] min_val the minimum value
         * \param[in] max_val the maximum value
         */
        void normalize_attribute(
            const std::string& name,
            float min_val = 0.0, float max_val = 1.0
        );

        /**
         * \brief Computes the distance between each vertex and a surface.
         * \param[in] surface the surface
         * \param[in] attribute the name of the vertex attribute
         * \param[in] signed_dist if true, computes the signed distance
         *  (needs the input shape to be tetrahedralized)
         */
        void compute_distance_to_surface(
            const MeshGrobName& surface,
            const std::string& attribute="distance",
            bool signed_dist=false
        );

        /**
         * \brief Reconstructs a surface from points and normals using
         *  Misha Kahzdan's Screened Poisson Reconstruction.
         * \param[in] points the name of the object that stores the points.
         *  It needs to have a "normal" attribute.
         * \param[in] attribute the name of the attribute used to store the
         *  reconstructed implicit function.
         * \param[in] depth the depth of the octree, 8 is the default value,
         *  use 10 or 11 for highly detailed models
         * \param[in] reconstruction (optional) the name of the
         *  reconstructed surface
         */
        void Poisson_reconstruction(
            const MeshGrobName& points,
            const std::string& attribute="distance",
            index_t depth = 8,
            const NewMeshGrobName& reconstruction="reconstruction"
        );


	/**
	 * \brief Loads an attribute for a 32 bit floating point
	 *    raw file.
         * \param[in] filename a binary file with the attribute stored
         *    as single-precision floats.
	 */
	void load_attribute(
	    const std::string& attribute,
	    const FileName& filename
	);

       /*********************************************************************/

    };

}
#endif
