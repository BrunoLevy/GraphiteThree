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
 * As an exception to the GPL,
 *  Graphite can be linked with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */


#include <OGF/mesh_gfx/tools/mesh_grob_tool.h>
#include <OGF/mesh_gfx/shaders/mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>

#include <geogram/image/image_library.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/command_line.h>

namespace OGF {

    /*******************************************************************/

    MeshGrobTool::MeshGrobTool(ToolsManager* parent) :
        Tool(parent) {
    }

    MeshGrobTool::~MeshGrobTool() {
    }

    index_t MeshGrobTool::pick_vertex(const RayPick& rp) {
        index_t result = pick(rp, MESH_VERTICES);

	// The rest of this function:
	// Snap picked coordinates to the vertex, instead of using pixel
	// and depth coordinates, that may be wrong due to glyph rendering
	// that changes depth value and to point size.

	if(result == NO_INDEX) {
	    return result;
	}

	if(mesh_grob()->vertices.dimension() < 3) {
	    return result;
	}

	Shader* shd = dynamic_cast<Shader*>(object()->get_shader());
	if(shd == nullptr) {
	    return result;
	}

	GLdouble* modelview = shd->latest_modelview();
	GLdouble* project = shd->latest_project();
	GLint* viewport = shd->latest_viewport();
	if(modelview == nullptr || project == nullptr || viewport == nullptr) {
	    return result;
	}

	// picked point (world coordinates) is picked vertex
	picked_point_ = mesh_grob()->vertices.point(result);

	// re-compute picked depth by transforming picked point
	double x_screen,y_screen;
	glupProject(
	    picked_point_.x, picked_point_.y, picked_point_.z,
	    modelview, project, viewport,
	    &x_screen, &y_screen, &picked_depth_
	);

	// re-compute picked NDC coordinates by transforming picked point
	picked_ndc_ = rendering_context()->screen_to_ndc(
	    index_t(x_screen), index_t(y_screen)
	);
	picked_ndc_.y = -picked_ndc_.y; // I must confess I don't know why !

	return result;
    }

    index_t MeshGrobTool::pick_edge(const RayPick& rp) {
        return pick(rp, MESH_EDGES);
    }

    index_t MeshGrobTool::pick_facet(const RayPick& rp) {
        return pick(rp, MESH_FACETS);
    }

    index_t MeshGrobTool::pick_cell(const RayPick& rp) {
        return pick(rp, MESH_CELLS);
    }

    bool MeshGrobTool::pick_facet_edge(
        const RayPick& rp, index_t& facet, index_t& corner
    ) {
        facet = NO_INDEX;
        corner = NO_INDEX;
        if(mesh_grob()->vertices.dimension() < 3) {
            return false;
        }

        //   Step 1: pick a facet.

        facet = pick_facet(rp);
        if(facet == NO_INDEX || facet > mesh_grob()->facets.nb()) {
            return false;
        }

        //   Step 2: find among all edges of the facet the one that
        // is nearest to the picked point.

        vec3 picked_point = rendering_context()->picked_point();
        double best_distance = Numeric::max_float64();
        for(index_t c1: mesh_grob()->facets.corners(facet)) {
            index_t c2 = mesh_grob()->facets.next_corner_around_facet(facet,c1);
            index_t v1 = mesh_grob()->facet_corners.vertex(c1);
            index_t v2 = mesh_grob()->facet_corners.vertex(c2);
            vec3 p1(mesh_grob()->vertices.point_ptr(v1));
            vec3 p2(mesh_grob()->vertices.point_ptr(v2));
            double distance = Geom::point_segment_squared_distance(
                picked_point, p1, p2
            );
            if(distance < best_distance) {
                best_distance = distance;
                corner = c1;
            }
        }
        return true;
    }

    index_t MeshGrobTool::pick(
        const RayPick& rp, MeshElementsFlags what, Image* image,
        index_t x0, index_t y0, index_t width, index_t height
    ) {
        if(mesh_grob() == nullptr || mesh_grob()->vertices.dimension() < 3) {
            return NO_INDEX;
        }

        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );
        if(shd == nullptr) {
            return NO_INDEX;
        }

        rendering_context()->begin_picking(rp.p_ndc);
        rendering_context()->begin_frame();

        glupMultMatrix(focus());
        glupMultMatrix(mesh_grob()->get_obj_to_world_transform()) ;

        shd->pick(what);

        if(CmdLine::get_arg_bool("dbg:picking")) {
	    Logger::out("Tool") << "Saving pick_debug.png" << std::endl;
            Image_var dbg_image = new Image;

            rendering_context()->snapshot(
                dbg_image
            );
            ImageLibrary::instance()->save_image(
                "pick_debug.png",dbg_image
            );
        }

        if(image != nullptr) {
            rendering_context()->snapshot(image, true, x0, y0, width, height);
        }

        rendering_context()->end_frame();
        rendering_context()->end_picking();

        index_t result = rendering_context()->picked_id();

        picked_ndc_ = rp.p_ndc;
        picked_point_ = rendering_context()->picked_point();
        picked_depth_ = rendering_context()->picked_depth();

        return result;
    }

    vec3 MeshGrobTool::drag_point(const RayPick& rp) const {

        //   TODO: it's a bit stupid, we could do that without
        // going through begin_frame() / end_frame() by caching
        // the current viewport and modelview transforms...

        vec2 dragged_ndc = rp.p_ndc;
        rendering_context()->begin_picking(dragged_ndc);
        rendering_context()->begin_frame();

        glupMultMatrix(focus());
        glupMultMatrix(mesh_grob()->get_obj_to_world_transform()) ;

        vec3 result = rendering_context()->unproject(
            dragged_ndc, picked_depth_
        );
        rendering_context()->end_frame();
        rendering_context()->end_picking();
        return result;
    }

    /*******************************************************************/

    MeshGrobTransformSubset::MeshGrobTransformSubset(
        MeshGrobTransformTool* parent
    ) :
        MeshGrobTool(parent->tools_manager()),
        transform_tool_(parent) {
    }

    void MeshGrobTransformSubset::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        transform_tool_->pick_subset(this, p_ndc);
    }

    void MeshGrobTransformSubset::update_transform_subset(const mat4& M) {
        transform_tool_->update_transform_subset(M);
        mesh_grob()->update();
    }

    const vec3& MeshGrobTransformSubset::center() const {
        return transform_tool_->center();
    }

    /*******************/

    void MeshGrobMoveSubset::grab(const RayPick& p_ndc) {
        MeshGrobTransformSubset::grab(p_ndc);
    }

    void MeshGrobMoveSubset::drag(const RayPick& p_ndc) {
        vec3 dragged_point = drag_point(p_ndc);
        vec3 T = dragged_point - picked_point_;
        mat4 M = create_translation_matrix(T);
        update_transform_subset(M);
    }

    /*******************/

    void MeshGrobResizeSubset::grab(const RayPick& p_ndc) {
        MeshGrobTransformSubset::grab(p_ndc);
    }

    void MeshGrobResizeSubset::drag(const RayPick& p_ndc) {
        vec3 dest = drag_point(p_ndc) ;
        double R2 = length(dest - center()) ;
        double R1 = length(picked_point_ - center()) ;
        double s = (R1 > 1e-6) ? R2 / R1 : 1.0 ;
        mat4 M =
            create_translation_matrix(-center()) *
            create_scaling_matrix(s) *
            create_translation_matrix(center());
        update_transform_subset(M);
    }

    /*******************/

    void MeshGrobScrollResizeSubset::grab(const RayPick& p_ndc) {
        MeshGrobTransformSubset::grab(p_ndc);
        const double s = 1.0 + step_*0.05;
        const mat4 M =
            create_translation_matrix(-center()) *
            create_scaling_matrix(s) *
            create_translation_matrix(center());
        update_transform_subset(M);

    }

    /*******************/

    void MeshGrobRotateSubset::grab(const RayPick& rp) {
        mat4 M;
        M.load_identity();
        arc_ball_->set_value(M);
        arc_ball_->grab(vec2(0.0, 0.0));
        MeshGrobTransformSubset::grab(rp);
    }

    void MeshGrobRotateSubset::drag(const RayPick& rp) {
        arc_ball_->drag(-(rp.p_ndc - picked_ndc()));
        mat4 view = rendering_context()->viewing_matrix();
        mat4 M =
            create_translation_matrix(-center()) *
            view *
            arc_ball_->get_value().inverse() *
            view.inverse() *
            create_translation_matrix(center());

        update_transform_subset(M);
    }

    void MeshGrobRotateSubset::release(const RayPick& rp) {
        arc_ball_->release(rp.p_ndc);
    }

    /**********************************************/

    void MeshGrobTransformTool::grab(const RayPick& p_ndc) {
        clear_subset();
        prev_inverse_transform_.load_identity();
        MultiTool::grab(p_ndc);
    }

    void MeshGrobTransformTool::release(const RayPick& p_ndc) {
        MultiTool::release(p_ndc);
        clear_subset();
        prev_inverse_transform_.load_identity();
    }

    void MeshGrobTransformTool::clear_subset() {
    }

    /*******************************************************************/
}
