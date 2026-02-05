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


#ifndef H_OGF_WARPDRIVE_COMMANDS_VOXEL_GROB_TRANSPORT_COMMANDS_H
#define H_OGF_WARPDRIVE_COMMANDS_VOXEL_GROB_TRANSPORT_COMMANDS_H

#include <OGF/WarpDrive/common/common.h>
#include <OGF/voxel/commands/voxel_grob_commands.h>
#include <OGF/mesh/grob/mesh_grob.h>

namespace OGF {

   /**
    * \brief VoxelGrob commands for optimal transport.
    */
    gom_class WarpDrive_API VoxelGrobTransportCommands
	: public VoxelGrobCommands {
    public:

        /**
         * \brief VoxelGrobTransportCommands constructor.
         */
        VoxelGrobTransportCommands();

        /**
         * \brief VoxelGrobTransportCommands destructor.
         */
        ~VoxelGrobTransportCommands() override;

    gom_slots:

        /**
         * \brief Initializes a voxel grob from a pointset.
         * \param[in] points name of the pointset. It should have points
	 *   located on a grid.
         * \menu /Geometry
	 */
	void init_from_pointset(const MeshGrobName& points);

	/**
	 * \brief Interpolates an attribute by solving a Poisson equation
	 * \param[in] attribute name of the attribute
	 * \param[in] bkgnd_value background value for the attribute
	 * \param[in] margin_width number of voxels around data where
	 *  to interpolate attributes
	 */
	void interpolate_attribute(
	    const std::string& attribute,
	    double bkgnd_value, index_t margin_width
	);
    };


}

#endif
