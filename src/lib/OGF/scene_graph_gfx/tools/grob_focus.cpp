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

#include <OGF/scene_graph_gfx/tools/grob_focus.h>
#include <OGF/scene_graph_gfx/tools/tools_manager.h>
#include <OGF/scene_graph_gfx/tools/scene_graph_tools_manager.h>
#include <OGF/scene_graph_gfx/shaders/scene_graph_shader_manager.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/gom/reflection/meta_class.h>

#include <geogram/image/image_library.h>
#include <geogram/basic/command_line.h>


namespace OGF {

    //_________________________________________________________________________


    void GrobFocus::grab(const RayPick& rp) {

	if(rp.button != 1) {
	    return;
	}

        // Do not call Tool::grab(), no need to
	// save state to undo/redo buffers.

	if(rendering_context() == nullptr) {
	    return;
	}

        rendering_context()->begin_picking(rp.p_ndc);
        rendering_context()->begin_frame();
        SceneGraphShaderManager* sg_shd_mgr =
	    dynamic_cast<SceneGraphShaderManager*>(
		scene_graph()->get_scene_graph_shader_manager()
	    );
        if(sg_shd_mgr != nullptr) {
            sg_shd_mgr->pick_object();
        }
        rendering_context()->end_frame();
        rendering_context()->end_picking();

	if(rendering_context()->picked_id() == NO_INDEX) {
	    return;
	}

	vec3 p = rendering_context()->picked_point();

	Any xform_any = Interpreter::default_interpreter()->resolve("xform");
	Object* xform;
	xform_any.get_value(xform);

	if(xform == nullptr) {
	    return;
	}

	xform->set_property("look_at",String::to_string(p));
        object()->scene_graph()->update();
    }

}
