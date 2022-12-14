
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

#include <OGF/mesh/tools/mesh_grob_paint_tool.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <geogram/mesh/mesh_geometry.h>

namespace OGF {

    MeshGrobPaint::MeshGrobPaint(ToolsManager* parent) : MeshGrobTool(parent) {
       value_ = 1.0;
       accumulate_ = false;
    }
   
    void MeshGrobPaint::grab(const RayPick& p_ndc) {
        paint(p_ndc);
    }

    void MeshGrobPaint::drag(const RayPick& p_ndc) {
        paint(p_ndc);
    }

    void MeshGrobPaint::paint(const RayPick& p_ndc) {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );
        if(shd == nullptr) {
            return;
        }

        std::string painting;
        shd->get_property("painting",painting);
        if(painting != "ATTRIBUTE") {
            return;
        }

        std::string attribute;
        shd->get_property("attribute",attribute);
        std::string subelement_name;
        std::string attribute_name;
        String::split_string(
            attribute, '.',
            subelement_name,
            attribute_name
        );
        MeshElementsFlags subelement =
            mesh_grob()->name_to_subelements_type(subelement_name);

        Attribute<double> attr;
        switch(subelement) {
        case MESH_VERTICES:
            attr.bind_if_is_defined(
                mesh_grob()->vertices.attributes(), attribute_name
            );
            break;
        case MESH_EDGES:
            attr.bind_if_is_defined(
                mesh_grob()->edges.attributes(), attribute_name
            );
            break;
        case MESH_FACETS:
            attr.bind_if_is_defined(
                mesh_grob()->facets.attributes(), attribute_name
            );
            break;
        case MESH_CELLS:
            attr.bind_if_is_defined(
                mesh_grob()->cells.attributes(), attribute_name
            );
            break;
        default:
            break;
        }

        if(!attr.is_bound()) {
            return;
        }

        index_t picked_element = pick(p_ndc,subelement);

        if(picked_element != index_t(-1)) {
            if(accumulate_) {
                attr[picked_element] += (
                    p_ndc.button == 1 ? value_ : -value_
                );
                mesh_grob()->update();
                shd->invoke_method("autorange");
            } else {
                attr[picked_element] = (
                    p_ndc.button == 1 ? value_ : 0.0
                );
                mesh_grob()->update();
            }
        }
    }

    void MeshGrobPaint::reset() {
    }
   
}
