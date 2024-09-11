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

#include <OGF/scene_graph_gfx/tools/grob_pan.h>
#include <OGF/scene_graph_gfx/tools/tools_manager.h>
#include <OGF/scene_graph_gfx/tools/scene_graph_tools_manager.h>
#include <OGF/renderer/context/rendering_context.h>

namespace OGF {

    //_________________________________________________________________________

    // Note: for the moment, this tool just forwards the events to the rendering
    // context. It is quite ugly (but at least it works)

    void GrobPan::grab(const RayPick& r) {

        // Do not call Tool::grab(), no need to
	// save state to undo/redo buffers.

        Node* renderer = tools_manager()->manager()->renderer();
        RenderingContext* context =
            tools_manager()->manager()->rendering_context();
        ArgList args;
        args.create_arg("rendering_context", context);
        vec2 p_ndc(r.p_ndc);
        vec2 p_wc = transform_point(p_ndc, context->inverse_viewing_matrix());
        args.create_arg("point_ndc", p_ndc);
        args.create_arg("point_wc",  p_wc);
        args.create_arg("button",   r.button);
        args.create_arg("control",  true);
        args.create_arg("shift",    false);
        renderer->invoke_method("mouse_down", args);
        last_p_ndc_ = p_ndc;
        last_p_wc_  = p_wc;
    }

    void GrobPan::drag(const RayPick& r) {
        Node* renderer = tools_manager()->manager()->renderer();
        RenderingContext* context =
            tools_manager()->manager()->rendering_context();
        ArgList args;
        args.create_arg("rendering_context", context);
        vec2 p_ndc(r.p_ndc);
        vec2 p_wc = transform_point(p_ndc, context->inverse_viewing_matrix());
        args.create_arg("point_ndc", p_ndc);
        args.create_arg("point_wc",  p_wc);
        args.create_arg("button",    r.button);
        args.create_arg("control",   true);
        args.create_arg("shift",     false);

        vec2 delta_ndc = p_ndc - last_p_ndc_;
        vec2 delta_wc  = p_wc  - last_p_wc_;

        args.create_arg("delta_ndc", delta_ndc);
        args.create_arg("delta_x_ndc", delta_ndc.x);
        args.create_arg("delta_y_ndc", delta_ndc.y);
        args.create_arg("delta_wc", delta_wc);

        renderer->invoke_method("mouse_move", args);
        last_p_ndc_ = p_ndc;
        last_p_wc_  = p_wc;
    }

    void GrobPan::release(const RayPick& r) {
        Node* renderer = tools_manager()->manager()->renderer();
        RenderingContext* context =
            tools_manager()->manager()->rendering_context();
        ArgList args;
        args.create_arg("rendering_context", context);
        vec2 p_ndc(r.p_ndc);
        vec2 p_wc = transform_point(p_ndc, context->inverse_viewing_matrix());
        args.create_arg("point_ndc", p_ndc);
        args.create_arg("point_wc",  p_wc);
        args.create_arg("button",   r.button);
        args.create_arg("control",  true);
        args.create_arg("shift",    false);
        renderer->invoke_method("mouse_up", args);
    }

    void GrobPan::reset() {
        status("Left: pan;  Middle: zoom;  Right: rotate");
    }

    //_______________________________________________________________________

}
