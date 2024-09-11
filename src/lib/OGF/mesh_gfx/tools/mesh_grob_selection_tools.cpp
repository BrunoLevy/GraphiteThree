
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
 * As an exception to the GPL, Graphite can be linked
 *  with the following (non-GPL) libraries: Qt, SuperLU, WildMagic and CGAL
 */

#include <OGF/mesh_gfx/tools/mesh_grob_selection_tools.h>
#include <OGF/mesh_gfx/shaders/mesh_grob_shader.h>
#include <geogram/mesh/mesh_geometry.h>

namespace OGF {

    void MeshGrobSelectVertex::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        vertex_ = pick_vertex(p_ndc);
        if(vertex_ != NO_VERTEX) {
            Attribute<bool> v_selection(
                mesh_grob()->vertices.attributes(), "selection"
            );
            v_selection[vertex_] = true;
            mesh_grob()->update();
        }
    }

    void MeshGrobSelectVertex::drag(const RayPick& p_ndc) {
	if(
	    vertex_ != NO_VERTEX &&
	    vertex_ < mesh_grob()->vertices.nb() &&
	    mesh_grob()->vertices.dimension() >= 3
	) {
            Geom::mesh_vertex_ref(*mesh_grob(), vertex_)
                = drag_point(p_ndc);
            mesh_grob()->update();
	}
    }

    void MeshGrobUnselectVertex::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        index_t v = pick_vertex(p_ndc);
        if(v != NO_VERTEX) {
            Attribute<bool> v_selection(
                mesh_grob()->vertices.attributes(), "selection"
            );
            v_selection[v] = false;
            mesh_grob()->update();
        }
    }

    void MeshGrobSelectUnselectVertex::reset() {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            object()->get_shader()
        );
        if(shd != nullptr) {
            shd->show_mesh();
            shd->show_vertices();
            shd->show_vertices_selection();
        }
        MultiTool::reset();
    }
}
