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


#ifndef H_OGF_MESH_GFX_COMMANDS_MESH_GROB_VISIBILITY_COMMANDS_H
#define H_OGF_MESH_GFX_COMMANDS_MESH_GROB_VISIBILITY_COMMANDS_H

#include <OGF/mesh_gfx/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh_gfx/commands/mesh_grob_visibility_commands.h
 * \brief Commands that compute elements visibility
 */

namespace OGF {

   /**
    * \brief Commands that compute elements visibility
    * \details This class is in mesh_gfx rather than mesh because it needs to
    *  access the renderer and the graphics
    */
    gom_class MESH_GFX_API MeshGrobVisibilityCommands : public MeshGrobCommands {
    public:

        /**
         * \brief MeshGrobVisibilityCommands constructor.
         */
        MeshGrobVisibilityCommands();

	/**
	 * \brief MeshGrobVisibilityCommands destructor.
	 */
	 ~MeshGrobVisibilityCommands() override;

    gom_slots:

       /*********************************************************************/

        /**
         * \brief Computes facets visibility from random views.
         * \param[in] nb_views number of views
	 * \param[in] dual_sided if true, facets seen as backfacing
	 *  count as negative.
         * \menu /Attributes/Facets
         */
        void compute_facets_visibility(
	    index_t nb_views = 1000,
	    bool dual_sided = true
	);

       /*********************************************************************/
    };

}
#endif
