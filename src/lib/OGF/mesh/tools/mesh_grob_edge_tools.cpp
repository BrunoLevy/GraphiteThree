
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

#include <OGF/mesh/tools/mesh_grob_edge_tools.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <geogram/mesh/mesh_geometry.h>

namespace OGF {
    
    void MeshGrobCreateEdge::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        if(v1_ == NO_VERTEX) {
            v1_ = pick_vertex(p_ndc);
            if(v1_ == NO_VERTEX) {
                Logger::err("Tool") << "Did not pick first vertex" << std::endl;
            } else {
                Logger::out("Tool") << "Now you can pick the second vertex"
                                    << std::endl;
            }
            return;
        }

        v2_ = pick_vertex(p_ndc);
        if(v2_ == NO_VERTEX) {
            Logger::err("Tool")
                << "Did not pick second vertex (retry)" << std::endl;
            return;
        }

        if(v1_ == v2_) {
            Logger::err("Tool") << "Picked the same vertex twice" << std::endl;
            reset();
            return;
        }

	mesh_grob()->edges.create_edge(v1_,v2_);
        mesh_grob()->update();

        v1_ = NO_VERTEX;
        v2_ = NO_VERTEX;
    }

    void MeshGrobCreateEdge::reset() {
        v1_ = NO_VERTEX;
        v2_ = NO_VERTEX;
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            object()->get_shader()
        );
        if(shd != nullptr) {
            shd->show_vertices();
        }
        MeshGrobTool::reset();
        Logger::out("Tool") << "Pick the first vertex" << std::endl;
    }

    
}
