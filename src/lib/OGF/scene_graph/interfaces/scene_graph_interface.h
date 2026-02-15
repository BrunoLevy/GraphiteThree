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

#ifndef H_OGF_SCENE_GRAPH_INTERFACES_SCENE_GRAPH_INTERFACE_H
#define H_OGF_SCENE_GRAPH_INTERFACES_SCENE_GRAPH_INTERFACE_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/commands/commands.h>
#include <OGF/scene_graph/types/scene_graph.h>

namespace OGF {

    /**
     * \brief All SceneGraph modifications go through a SceneGraphInterface.
     * \details Calls are logged to the history if called
     *  from user interaction.
     */
    gom_class SCENE_GRAPH_API SceneGraphInterface : public Interface {
    public:
	/**
	 * \brief SceneGraphInterface constructor
	 */
	SceneGraphInterface();

	/**
	 * \brief SceneGraphInterface destructor
	 */
	~SceneGraphInterface() override;

        /**
         * \copydoc Object::invoke_method
         * \details Overload of the invokation mechanism,
         *  that adds timings and history recording.
         */
        bool invoke_method(
            const std::string& method_name,
            const ArgList& args, Any& ret_val
        ) override;
    };

}


#endif
