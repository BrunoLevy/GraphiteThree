
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

namespace {
    using namespace OGF;

    /**
     * \brief A painting operation.
     */
    enum PaintOp {
        PAINT_SET,   /**< sets attribute value */
        PAINT_RESET, /**< resets attribute value to zero */
        PAINT_INC,   /**< adds to attribute value */
        PAINT_DEC    /**< subtracts from attribute value */
    };

    /**
     * \brief Paints an attribute value for a given type
     * \tparam T the type of the attribute
     * \param[in] mesh_grob a pointer to the MeshGrob
     * \param[in] where one of MESH_VERTICES, MESH_EDGES, MESH_FACETS or 
     *  MESH_CELLS
     * \param[in] name the name of the attribute
     * \param[in] element_id the element to be painted
     * \param[in] op one of PAINT_SET, PAINT_RESET, PAINT_INC or PAINT_DEC
     * \param[in] value the value to be painted
     * \retval true if an attribute of the specified type was found 
     * \retval false otherwise
     */
    template <class T> bool paint_attribute_generic(
        MeshGrob* mesh_grob, MeshElementsFlags where, const std::string& name,
        index_t element_id, PaintOp op, double value
    ) {
        MeshSubElementsStore& elts = mesh_grob->get_subelements_by_type(where);
        if(!Attribute<T>::is_defined(elts.attributes(), name)) {
            return false;
        }
        Attribute<T> attr(elts.attributes(), name);
        switch(op) {
        case PAINT_SET:
            attr[element_id] = T(value);
            break;
        case PAINT_RESET:
            attr[element_id] = T(0);
            break;            
        case PAINT_INC:
            attr[element_id] = attr[element_id] + T(value);
            break;            
        case PAINT_DEC:
            attr[element_id] = attr[element_id] - T(value);
            break;            
        }
        return true;
    }

    /**
     * \brief Paints an attribute value independently from the type. Tries
     *   int32, uint32, float, double
     * \details The specified value is converted into the type. Floating-point
     *   values are truncated if the attribute has integer type.
     * \param[in] mesh_grob a pointer to the MeshGrob
     * \param[in] where one of MESH_VERTICES, MESH_EDGES, MESH_FACETS or 
     *  MESH_CELLS
     * \param[in] name the name of the attribute
     * \param[in] element_id the element to be painted
     * \param[in] op one of PAINT_SET, PAINT_RESET, PAINT_INC or PAINT_DEC
     * \param[in] value the value to be painted
     * \retval true if an attribute could be painted
     * \retval false otherwise
     */
    bool paint_attribute(
        MeshGrob* mesh_grob, MeshElementsFlags where, const std::string& name,
        index_t element_id, PaintOp op, double value
    ) {
        return paint_attribute_generic<double>(
                   mesh_grob, where, name, element_id, op, value
               ) ||
               paint_attribute_generic<float>(
                   mesh_grob, where, name, element_id, op, value
               ) ||
               paint_attribute_generic<Numeric::uint32>(
                   mesh_grob, where, name, element_id, op, value
               ) ||
               paint_attribute_generic<Numeric::int32>(
                   mesh_grob, where, name, element_id, op, value
               ) ||
               paint_attribute_generic<bool>(
                   mesh_grob, where, name, element_id, op, value
               );            
    }
}

namespace OGF {

    MeshGrobPaint::MeshGrobPaint(ToolsManager* parent) : MeshGrobTool(parent) {
       value_ = 1.0;
       accumulate_ = false;
       autorange_ = true;
       pick_vertices_only_ = false;
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

        PaintOp op = PAINT_SET;
        if(accumulate_) {
            op = p_ndc.button == 1 ? PAINT_INC : PAINT_DEC;
        } else {
            op = p_ndc.button == 1 ? PAINT_SET : PAINT_RESET;
        }

        index_t picked_element = pick(p_ndc,subelement);
        if(picked_element != index_t(-1)) {
            paint_attribute(
                mesh_grob(), subelement, attribute_name,
                picked_element, op, value_
            );
        } else if(subelement == MESH_VERTICES && !pick_vertices_only_) {
            index_t f = pick_facet(p_ndc);
            if(f != index_t(-1)) {
                for(index_t lv = 0;
                    lv<mesh_grob()->facets.nb_vertices(f); ++lv
                ) {
                    index_t v = mesh_grob()->facets.vertex(f,lv);
                    paint_attribute(
                        mesh_grob(), subelement, attribute_name,
                        v, op, value_
                    );
                }
            } else {
                index_t c = pick_cell(p_ndc);
                if(c != index_t(-1)) {
                    for(index_t lv = 0;
                        lv<mesh_grob()->cells.nb_vertices(c); ++lv
                    ) {
                        index_t v = mesh_grob()->cells.vertex(c,lv);
                        paint_attribute(
                            mesh_grob(), subelement, attribute_name,
                            v, op, value_
                        );
                    }
                }
            }
        }
        
        if(autorange_) {
            shd->invoke_method("autorange");
        }
        
        mesh_grob()->update();
    }

    void MeshGrobPaint::reset() {
    }

    void MeshGrobPaint::set_pick_vertices_only(bool value) {
        pick_vertices_only_ = value;
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );
        if(shd != nullptr) {
            if(pick_vertices_only_) {
                shd->show_vertices();
            } else {
                shd->hide_vertices();                    
            }
        }
    }
    
}
