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

#include <OGF/scene_graph/tools/grob_light.h>
#include <OGF/scene_graph/tools/tools_manager.h>
#include <OGF/scene_graph/types/scene_graph_tools_manager.h>
#include <OGF/renderer/context/rendering_context.h>

namespace OGF {

    //_________________________________________________________________________

    // Note: for the moment, this tool just forwards the events to the rendering
    // context. It is quite ugly (but at least it works)

    void GrobLight::grab(const RayPick& r) {

        // Do not call Tool::grab(), no need to
	// save state to undo/redo buffers.
       
        RenderingContext* context =
            tools_manager()->manager()->rendering_context();

	arc_ball_->set_value(context->lighting_matrix());
	arc_ball_->grab(r.p_ndc);
    }
    
    void GrobLight::drag(const RayPick& r) {
        Node* renderer = tools_manager()->manager()->renderer();
        RenderingContext* context =
            tools_manager()->manager()->rendering_context();
	arc_ball_->drag(r.p_ndc);
	context->set_lighting_matrix(arc_ball_->get_value());
	renderer->invoke_method("update");
    }
    
    void GrobLight::release(const RayPick& r) {
        Node* renderer = tools_manager()->manager()->renderer();	
        RenderingContext* context =
            tools_manager()->manager()->rendering_context();
	arc_ball_->release(r.p_ndc);
	context->set_lighting_matrix(arc_ball_->get_value());
	renderer->invoke_method("update");	
    }

    //_______________________________________________________________________

}
