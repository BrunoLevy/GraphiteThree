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

#include <OGF/scene_graph_gfx/tools/grob_select.h>
#include <OGF/scene_graph_gfx/shaders/scene_graph_shader_manager.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/renderer/context/rendering_context.h>

#include <geogram/image/image_library.h>
#include <geogram/basic/command_line.h>



namespace OGF {

    //_________________________________________________________________________

    void GrobSelect::grab(const RayPick& rp) {

        // Do not call Tool::grab(), no need to
	// save state to undo/redo buffers.

        rendering_context()->begin_picking(rp.p_ndc);
        rendering_context()->begin_frame();

        SceneGraphShaderManager* sg_shd_mgr =
	    dynamic_cast<SceneGraphShaderManager*>(
		scene_graph()->get_scene_graph_shader_manager()
	    );

        if(sg_shd_mgr != nullptr) {
            sg_shd_mgr->pick_object();
        }

        if(CmdLine::get_arg_bool("dbg:picking")) {
	    Logger::out("Tool") << "Saving pick_debug.png" << std::endl;
            Image image;
            rendering_context()->snapshot(&image);
            ImageLibrary::instance()->save_image("pick_debug.png",&image);
        }

        rendering_context()->end_frame();
        rendering_context()->end_picking();
        index_t id = rendering_context()->picked_id();
        SceneGraph* sg = object()->scene_graph();

	{

	    Object* obj = nullptr;
	    if(id != index_t(-1) && id < sg->get_nb_children()) {
		obj = sg->ith_child(id);
		sg->set_current_object(sg->ith_child(id)->name());
	    }

	    Object* main = scene_graph()->get_application();
	    if(
		main != nullptr &&
		main->meta_class()->find_property("picked_grob") != nullptr
	    ) {
		// For context menu.
		Any value;
		value.set_value(obj);
		main->set_property("picked_grob", value);
	    }
	}

        //   Trigger a scene redraw, because the rendering context
        // contains the picking image.
        sg->update();

    }

    //_______________________________________________________________________

    void GrobSelect::release(const RayPick& value) {
	geo_argused(value);
    }

}
