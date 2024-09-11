/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
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
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 */

#ifndef H_OGF_SCENE_GRAPH_GFX_TOOLS_GROB_LIGHT_H
#define H_OGF_SCENE_GRAPH_GFX_TOOLS_GROB_LIGHT_H

#include <OGF/scene_graph_gfx/common/common.h>
#include <OGF/scene_graph_gfx/tools/tool.h>
#include <OGF/scene_graph_gfx/transforms/arc_ball.h>

/**
 * \file OGF/scene_graph_gfx/tools/grob_pan.h
 * \brief Implementation of the panning tool.
 */

namespace OGF {

    /**
     * \brief The camera panning tool.
     */
    gom_attribute(category,"viewer")
    gom_attribute(icon,"light")
    gom_attribute(help, "Edit light direction")
    gom_attribute(message, "btn1: edit light direction")
    gom_class SCENE_GRAPH_GFX_API GrobLight : public Tool {
    public:
        /**
         * \brief GrobLight constructor.
         * \param[in] parent a pointer to the ToolsManager
         */
        GrobLight(ToolsManager* parent) : Tool(parent) {
	    arc_ball_ = new ArcBall();
        }

        /**
         * \copydoc Tool::grab()
         */
         void grab(const RayPick& value) override;

        /**
         * \copydoc Tool::drag()
         */
	 void drag(const RayPick& value) override;

        /**
         * \copydoc Tool::release()
         */
	 void release(const RayPick& value) override;

    private:
	 ArcBall_var arc_ball_;
    };

}

#endif
