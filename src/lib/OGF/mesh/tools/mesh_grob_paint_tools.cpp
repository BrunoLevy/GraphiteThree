
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

#include <OGF/mesh/tools/mesh_grob_paint_tools.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <geogram_gfx/third_party/imgui/imgui.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/image/image_library.h>
#include <geogram/basic/stopwatch.h>

#include <geogram_gfx/third_party/imgui/imgui.h>

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
     * \param[in] component the component of a vector attribute 
     *            (0 for a scalar attribute)
     * \param[in] op one of PAINT_SET, PAINT_RESET, PAINT_INC or PAINT_DEC
     * \param[in] value the value to be painted
     * \retval true if an attribute of the specified type was found 
     * \retval false otherwise
     */
    template <class T> bool paint_attribute_generic(
        MeshGrob* mesh_grob, MeshElementsFlags where,
        const std::string& name, index_t component,
        index_t element_id, PaintOp op, double value
    ) {
        MeshSubElementsStore& elts = mesh_grob->get_subelements_by_type(where);
        if(!Attribute<T>::is_defined(elts.attributes(), name)) {
            return false;
        }
        Attribute<T> attr(elts.attributes(), name);

        switch(op) {
        case PAINT_SET:
            attr[element_id*attr.dimension()+component] = T(value);
            break;
        case PAINT_RESET:
            attr[element_id*attr.dimension()+component] = T(0);
            break;            
        case PAINT_INC:
            attr[element_id*attr.dimension()+component] =
                attr[element_id*attr.dimension()+component] + T(value);
            break;            
        case PAINT_DEC:
            attr[element_id*attr.dimension()+component] =
                attr[element_id*attr.dimension()+component] - T(value);
            break;            
        }
        return true;
    }

    /**
     * \brief Paints an attribute value independently from the type. Tries
     *   int32, uint32, float, double, bool
     * \details The specified value is converted into the type. Floating-point
     *   values are truncated if the attribute has integer type.
     * \param[in] mesh_grob a pointer to the MeshGrob
     * \param[in] where one of MESH_VERTICES, MESH_EDGES, MESH_FACETS or 
     *  MESH_CELLS
     * \param[in] name the name of the attribute
     * \param[in] element_id the element to be painted
     * \param[in] component the component of a vector attribute 
     *            (0 for a scalar attribute)
     * \param[in] op one of PAINT_SET, PAINT_RESET, PAINT_INC or PAINT_DEC
     * \param[in] value the value to be painted
     * \retval true if an attribute could be painted
     * \retval false otherwise
     */
    bool paint_attribute(
        MeshGrob* mesh_grob, MeshElementsFlags where,
        const std::string& name, index_t component,
        index_t element_id, PaintOp op, double value
    ) {
        return paint_attribute_generic<double>(
                   mesh_grob, where, name, component, element_id, op, value
               ) ||
               paint_attribute_generic<float>(
                   mesh_grob, where, name, component, element_id, op, value
               ) ||
               paint_attribute_generic<Numeric::uint32>(
                   mesh_grob, where, name, component, element_id, op, value
               ) ||
               paint_attribute_generic<Numeric::int32>(
                   mesh_grob, where, name, component, element_id, op, value
               ) ||
               paint_attribute_generic<bool>(
                   mesh_grob, where, name, component, element_id, op, value
               );            
    }

    /**
     * \brief Probes an attribute value for a given type
     * \tparam T the type of the attribute
     * \param[in] mesh_grob a pointer to the MeshGrob
     * \param[in] where one of MESH_VERTICES, MESH_EDGES, MESH_FACETS or 
     *  MESH_CELLS
     * \param[in] name the name of the attribute
     * \param[in] element_id the element probe
     * \param[in] component the component of a vector attribute 
     *            (0 for a scalar attribute)
     * \param[out] value the probed value
     * \retval true if an attribute of the specified type was found 
     * \retval false otherwise
     */
    template <class T> bool probe_attribute_generic(
        MeshGrob* mesh_grob, MeshElementsFlags where,
        const std::string& name, index_t component,
        index_t element_id, double& value
    ) {
        MeshSubElementsStore& elts = mesh_grob->get_subelements_by_type(where);
        if(!Attribute<T>::is_defined(elts.attributes(), name)) {
            return false;
        }
        Attribute<T> attr(elts.attributes(), name);
        value = double(attr[element_id*attr.dimension()+component]);
        return true;
    }

    /**
     * \brief Paints an attribute value independently from the type. Tries
     *   int32, uint32, float, double, bool
     * \details The specified value is converted into double. Booleans are
     *   converted into 1.0 (true) or 0.0 (false)
     * \param[in] mesh_grob a pointer to the MeshGrob
     * \param[in] where one of MESH_VERTICES, MESH_EDGES, MESH_FACETS or 
     *  MESH_CELLS
     * \param[in] name the name of the attribute
     * \param[in] element_id the element 
     * \param[in] component the component of a vector attribute 
     *            (0 for a scalar attribute)
     * \param[out] value the value of the probed attribute
     * \retval true if an attribute could be probed
     * \retval false otherwise
     */
    bool probe_attribute(
        MeshGrob* mesh_grob, MeshElementsFlags where,
        const std::string& name, index_t component,
        index_t element_id, double& value
    ) {
        return probe_attribute_generic<double>(
                   mesh_grob, where, name, component, element_id, value
               ) ||
               probe_attribute_generic<float>(
                   mesh_grob, where, name, component, element_id, value
               ) ||
               probe_attribute_generic<Numeric::uint32>(
                   mesh_grob, where, name, component, element_id, value
               ) ||
               probe_attribute_generic<Numeric::int32>(
                   mesh_grob, where, name, component, element_id, value
               ) ||
               probe_attribute_generic<bool>(
                   mesh_grob, where, name, component, element_id, value
               );            
    }

    /**
     * \brief Probes an interpolated vertex attribute in a mesh facet
     * \param[in] mesh_grob a pointer to the mesh
     * \param[in] name the name of the vertex attribute (without the prefix
     *  "vertices." and without the suffix "[component]")
     * \param[in] component the component or 0 if it is a scalar attribute
     * \param[in] q the point where the attribute should be interpolated
     * \return the interpolated attribute
     */
    double probe_vertex_attribute_in_facet(
        MeshGrob* mesh_grob,
        const std::string& name, index_t component,
        index_t f, vec3 q
    ) {
        // Virtually triangulate the facet
        index_t c1 = mesh_grob->facets.corners_begin(f);
        index_t v1 = mesh_grob->facet_corners.vertex(c1);
        vec3 p1(mesh_grob->vertices.point_ptr(v1));
        for(
            index_t c2 = c1+1;
            c2+1 < mesh_grob->facets.corners_end(f); ++c2
        ) {
            index_t c3=c2+1;
            index_t v2 = mesh_grob->facet_corners.vertex(c2);
            index_t v3 = mesh_grob->facet_corners.vertex(c3);
            vec3 p2(mesh_grob->vertices.point_ptr(v2));
            vec3 p3(mesh_grob->vertices.point_ptr(v3));

            // Barycentric coordinates of q in triangle p1,p2,p3
            double A =  GEO::Geom::triangle_area(p1,p2,p3);
            double l1 = GEO::Geom::triangle_area(q,p2,p3);
            double l2 = GEO::Geom::triangle_area(p1,q,p3);
            double l3 = GEO::Geom::triangle_area(p1,p2,q);

            // Normally l1+l2+l3 should be exactly equal to A
            // for the triangle that contains q.
            // We let a tolerance (probed point may be not
            // exactly on facet, and facet may be non-planar).
            if(A > 1e-6 && l1+l2+l3 <= A*1.1) {
                double a1,a2,a3;
                probe_attribute(
                    mesh_grob, MESH_VERTICES, name, component, v1, a1
                );
                probe_attribute(
                    mesh_grob, MESH_VERTICES, name, component, v2, a2
                );
                probe_attribute(
                    mesh_grob, MESH_VERTICES, name, component, v3, a3
                );
                return (l1*a1+l2*a2+l3*a3)/A;
            }
        }
        index_t N = mesh_grob->facets.nb_vertices(f);
        double result=0.0;
        for(index_t lv=0; lv<N; ++lv) {
            index_t v = mesh_grob->facets.vertex(f,lv);
            double a;
            probe_attribute(
                mesh_grob, MESH_VERTICES, name, component, v, a
            );
            result += a;
        }
        result /= double(N);
        return result;
    }


    /**
     * \brief Probes an interpolated vertex attribute in a mesh cell
     * \param[in] mesh_grob a pointer to the mesh
     * \param[in] name the name of the vertex attribute (without the prefix
     *  "vertices." and without the suffix "[component]")
     * \param[in] component the component or 0 if it is a scalar attribute
     * \param[in] q the point where the attribute should be interpolated
     * \return the interpolated attribute
     */
    double probe_vertex_attribute_in_cell(
        MeshGrob* mesh_grob,
        const std::string& name, index_t component,
        index_t c, vec3 q
    ) {

        // Virtually tetrahedralize the cell
        
        index_t v1 = mesh_grob->cells.facet_vertex(c,0,0);
        vec3 p1(mesh_grob->vertices.point_ptr(v1));
        
        for(index_t lf=0; lf<mesh_grob->cells.nb_facets(c); ++lf) {
            index_t v2 = mesh_grob->cells.facet_vertex(c,lf,0);
            vec3 p2(mesh_grob->vertices.point_ptr(v2));            
            if(v2 == v1) {
                continue;
            }
            for(
                index_t lv=1;lv+1 < mesh_grob->cells.facet_nb_vertices(c,lf);
                ++lv
            ) {
                index_t v3 = mesh_grob->cells.facet_vertex(c,lf,lv);
                index_t v4 = mesh_grob->cells.facet_vertex(c,lf,lv+1);

                if(v3 == v1 || v3 == v2 || v4 == v1 || v4 == v2) {
                    continue;
                }

                vec3 p3(mesh_grob->vertices.point_ptr(v3));
                vec3 p4(mesh_grob->vertices.point_ptr(v4));                

                // Baryentric coordinates of q in p1,p2,p3,p4
                double V  = GEO::Geom::tetra_volume(p1,p2,p3,p4);
                double l1 = GEO::Geom::tetra_volume(q,p2,p3,p4);
                double l2 = GEO::Geom::tetra_volume(p1,q,p3,p4);
                double l3 = GEO::Geom::tetra_volume(p1,p2,q,p4);
                double l4 = GEO::Geom::tetra_volume(p1,p2,p3,q);            
            
                // Normally l1+l2+l3+l4 should be exactly equal to V for
                // the tet that contains q. We let a tolerance.
                if(V > 1e-6 && l1+l2+l3+l4 <= V*1.1) {
                    double a1,a2,a3,a4;
                    probe_attribute(
                        mesh_grob, MESH_VERTICES, name, component, v1, a1
                    );
                    probe_attribute(
                        mesh_grob, MESH_VERTICES, name, component, v2, a2
                    );
                    probe_attribute(
                        mesh_grob, MESH_VERTICES, name, component, v3, a3
                    );
                    probe_attribute(
                        mesh_grob, MESH_VERTICES, name, component, v4, a4
                    );
                    return (l1*a1+l2*a2+l3*a3+l4*a4)/V;
                }
            }
        }
        index_t N = mesh_grob->cells.nb_vertices(c);
        double result=0.0;
        for(index_t lv=0; lv<N; ++lv) {
            index_t v = mesh_grob->cells.vertex(c,lv);
            double a;
            probe_attribute(
                mesh_grob, MESH_VERTICES, name, component, v, a
            );
            result += a;
        }
        result /= double(N);
        return result;
    }

    void for_each_picked_element(
        Image* picking_image, std::function<void(index_t)> doit
    ) {
        // Get all the picked ids, by sorting the pixels of the image by
        // value then using std::unique
        Numeric::uint32* begin = (Numeric::uint32*)(picking_image->base_mem());
        Numeric::uint32* end =
            begin + picking_image->width() * picking_image->height();
        std::sort(begin, end);
        end = std::unique(begin,end);
        for(index_t* p=begin; p!=end; ++p) {
            if(*p != index_t(-1)) {
                doit(*p);
            }
        }
    }
}

namespace OGF {

    MeshGrobPaintTool::MeshGrobPaintTool(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {
       value_ = 1.0;
       accumulate_ = false;
       autorange_ = true;
       pick_vertices_only_ = false;
       timestamp_ = 0.0;
       update_time_ = 0.0;
       picked_element_ = index_t(-1);
    }

    void MeshGrobPaintTool::paint(const RayPick& p_ndc) {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );
        
        if(shd == nullptr) {
            return;
        }

        {
            std::string painting;
            shd->get_property("painting",painting);
            if(painting != "ATTRIBUTE") {
                return;
            }
        }

        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        {
            std::string full_attribute_name;
            shd->get_property("attribute",full_attribute_name);
            if(!Mesh::parse_attribute_name(
                   full_attribute_name,where,attribute_name,component)
            ) {
                return;
            }
        }
        
        PaintOp op = PAINT_SET;
        if(accumulate_) {
            op = p_ndc.button == 1 ? PAINT_INC : PAINT_DEC;
        } else {
            op = p_ndc.button == 1 ? PAINT_SET : PAINT_RESET;
        }

        index_t picked_element = pick(p_ndc,where);
        if(picked_element != index_t(-1)) {
            paint_attribute(
                mesh_grob(), where,
                attribute_name, component,
                picked_element, op, value_
            );
        } else if(where == MESH_VERTICES && !pick_vertices_only_) {
            index_t f = pick_facet(p_ndc);
            if(f != index_t(-1)) {
                picked_element_ = f;
                for(index_t lv = 0;
                    lv<mesh_grob()->facets.nb_vertices(f); ++lv
                ) {
                    index_t v = mesh_grob()->facets.vertex(f,lv);
                    paint_attribute(
                        mesh_grob(), where,
                        attribute_name, component,
                        v, op, value_
                    );
                } 
            } else {
                index_t c = pick_cell(p_ndc);
                picked_element_ = c;
                if(c != index_t(-1)) {
                    for(index_t lv = 0;
                        lv<mesh_grob()->cells.nb_vertices(c); ++lv
                    ) {
                        index_t v = mesh_grob()->cells.vertex(c,lv);
                        paint_attribute(
                            mesh_grob(), where,
                            attribute_name, component,
                            v, op, value_
                        );
                    }
                }
            }
        }

        if(picked_element != picked_element_) {
            if(autorange_) {
                shd->invoke_method("autorange");
            }
            mesh_grob()->update();
            picked_element_ = picked_element;
        }
    }

    void MeshGrobPaintTool::set_pick_vertices_only(bool value) {
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
    
    /**********************************************************************/
    
    MeshGrobPaint::MeshGrobPaint(
        ToolsManager* parent
    ) : MeshGrobPaintTool(parent) {
    }
   
    void MeshGrobPaint::grab(const RayPick& p_ndc) {
        latest_ndc_ = p_ndc.p_ndc;
        paint(p_ndc);
    }

    void MeshGrobPaint::drag(const RayPick& p_ndc) {
        if(length(p_ndc.p_ndc - latest_ndc_) <= 10.0/1024.0) {
            return ;
        }
        latest_ndc_ = p_ndc.p_ndc;
        paint(p_ndc);
    }

    void MeshGrobPaint::release(const RayPick& p_ndc) {
        paint(p_ndc);
    }

    /**********************************************************************/
    
    MeshGrobPaintRect::MeshGrobPaintRect(
        ToolsManager* parent
    ) : MeshGrobPaintTool(parent) {
        active_ = false;
    }
   
    void MeshGrobPaintRect::grab(const RayPick& p_ndc) {
        p_ = ndc_to_dc(p_ndc.p_ndc);
        active_ = true;
    }

    void MeshGrobPaintRect::drag(const RayPick& p_ndc) {
        if(!active_) {
            return;
        }
        vec2 q = ndc_to_dc(p_ndc.p_ndc);
        rendering_context()->overlay().clear();
        rendering_context()->overlay().fillrect(
            p_,q,Color(1.0, 1.0, 1.0, 0.5)
        );
        rendering_context()->overlay().rect(
            p_,q,Color(1.0, 1.0, 1.0, 1.0),1.5
        );

        rendering_context()->overlay().fillcircle(
            p_, 4.0, Color(1.0, 1.0, 1.0, 1.0)                
        );

        rendering_context()->overlay().fillcircle(
            q, 4.0, Color(1.0, 1.0, 1.0, 1.0)                
        );

        rendering_context()->overlay().fillcircle(
            vec2(p_.x, q.y), 4.0, Color(1.0, 1.0, 1.0, 1.0)                
        );

        rendering_context()->overlay().fillcircle(
            vec2(q.x,p_.y), 4.0, Color(1.0, 1.0, 1.0, 1.0)                
        );
        
    }

    void MeshGrobPaintRect::release(const RayPick& raypick) {
        if(!active_) {
            return;
        }
        active_ = false;
        vec2 q = ndc_to_dc(raypick.p_ndc);
        index_t px = index_t(p_.x);
        index_t py = index_t(p_.y);
        index_t qx = index_t(q.x);
        index_t qy = index_t(q.y);

        index_t x0 = std::min(px,qx);
        index_t y0 = std::min(py,qy);
        index_t x1 = std::max(px,qx);
        index_t y1 = std::max(py,qy);

        paint_rect(raypick,x0,y0,x1,y1);
        
        rendering_context()->overlay().clear();
    }

    void MeshGrobPaintRect::paint_rect(
        const RayPick& raypick,
        index_t x0, index_t y0, index_t x1, index_t y1
    ) {

        PaintOp op = PAINT_SET;
        if(accumulate_) {
            op = raypick.button == 1 ? PAINT_INC : PAINT_DEC;
        } else {
            op = raypick.button == 1 ? PAINT_SET : PAINT_RESET;
        }

        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );
        
        if(shd == nullptr) {
            return;
        }

        {
            std::string painting;
            shd->get_property("painting",painting);
            if(painting != "ATTRIBUTE") {
                return;
            }
        }

        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        {
            std::string full_attribute_name;
            shd->get_property("attribute",full_attribute_name);
            if(!Mesh::parse_attribute_name(
                   full_attribute_name,where,attribute_name,component)
            ) {
                return;
            }
        }

        // Pick the elements, and copy the selected rect in an image
        index_t width  = x1-x0+1;
        index_t height = y1-y0+1;
        Image_var picking_image = new Image;
        // We need 32-bit pixel values (default is 24-bit)
        picking_image->initialize(
            Image::RGBA, Image::BYTE, width, height
        );

        // Damnit, for glReadPixels, Y is swapped
        y0 = rendering_context()->get_height()-height-1-y0;
        
        pick(
            raypick, where, picking_image,
            x0, y0, width, height
        );

        for_each_picked_element(
            picking_image,
            [&](index_t picked_element) {
                paint_attribute(
                    mesh_grob(), where,
                    attribute_name, component,
                    picked_element, op, value_
                );
            }
        );
        
        if(!pick_vertices_only_ && where == MESH_VERTICES) {
            pick(raypick, MESH_FACETS, picking_image, x0, y0, width, height);
            for_each_picked_element(
                picking_image,
                [&](index_t f) {
                    for(index_t lv = 0;
                        lv<mesh_grob()->facets.nb_vertices(f); ++lv
                       ) {
                        index_t v = mesh_grob()->facets.vertex(f,lv);
                        paint_attribute(
                            mesh_grob(), where,
                            attribute_name, component,
                            v, op, value_
                        );
                    }
                }
            );

            pick(raypick, MESH_CELLS, picking_image, x0, y0, width, height);
            for_each_picked_element(
                picking_image,
                [&](index_t c) {
                    for(index_t lv = 0;
                        lv<mesh_grob()->cells.nb_vertices(c); ++lv
                    ) {
                        index_t v = mesh_grob()->cells.vertex(c,lv);
                        paint_attribute(
                            mesh_grob(), where,
                            attribute_name, component,
                            v, op, value_
                        );
                    }
                }
            );
        }
    }
    
    /***************************************************************/

    MeshGrobProbe::MeshGrobProbe(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {
        picked_ = false;
    }
   
    void MeshGrobProbe::grab(const RayPick& p_ndc) {
        latest_ndc_ = p_ndc.p_ndc;
        probe(p_ndc);
    }

    void MeshGrobProbe::drag(const RayPick& p_ndc) {
        if(length(p_ndc.p_ndc - latest_ndc_) <= 10.0/1024.0) {
            return ;
        }
        latest_ndc_ = p_ndc.p_ndc;
        probe(p_ndc);
    }

    void MeshGrobProbe::probe(const RayPick& p_ndc) {
        picked_ = false;
        
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );
        
        if(shd == nullptr) {
            return;
        }

        bool with_attributes = true;
        {
            std::string painting;
            shd->get_property("painting",painting);
            if(painting != "ATTRIBUTE") {
                with_attributes = false;
            }
        }


        MeshElementsFlags attribute_element_type;
        std::string attribute_name;
        index_t component;
        if(with_attributes) {
            std::string full_attribute_name;
            shd->get_property("attribute",full_attribute_name);
            if(!Mesh::parse_attribute_name(
                   full_attribute_name,
                   attribute_element_type,attribute_name,component
               )
            ) {
                return;
            }
        }
        
        double value;
        bool with_value = false;
        index_t element_id = index_t(-1);
        MeshElementsFlags element_type = MESH_NONE;
        vec3 p;

        static MeshElementsFlags element_types[3] = {
            MESH_VERTICES, MESH_FACETS, MESH_CELLS
        };

        for(index_t i=0; i<3; ++i) {
            if(element_id == index_t(-1)) { 
                element_id = pick(p_ndc, element_types[i]);
                if(element_id != index_t(-1)) {
                    p = picked_point();
                    element_type = element_types[i];                    
                    if(with_attributes &&
                       attribute_element_type == element_types[i]
                    ) {
                        with_value = probe_attribute(
                            mesh_grob(), element_types[i],
                            attribute_name, component,
                            element_id, value
                        );
                    }
                }
            }
        }

        if(
            with_attributes &&
            !with_value    &&
            attribute_element_type == MESH_VERTICES &&
            element_id != index_t(-1)
        ) {
            if(element_type == MESH_FACETS) {
                value = probe_vertex_attribute_in_facet(
                    mesh_grob(), attribute_name, component, element_id, p
                );
                with_value = true;
            } else if(element_type == MESH_CELLS) {
                value = probe_vertex_attribute_in_cell(
                    mesh_grob(), attribute_name, component, element_id, p
                );
                with_value = true;
            }
        }

        bool is_bool = false;
        index_t is_integer = false;
        index_t dim = 1;
        if(with_value) {
            AttributeStore* store =
                mesh_grob()->get_subelements_by_type(attribute_element_type)
                .attributes().find_attribute_store(attribute_name);

            dim = store->dimension();
            
            is_bool = store->elements_type_matches(
                typeid(Numeric::uint8).name()
            );

            is_integer = store->elements_type_matches(
                typeid(Numeric::uint32).name()
            ) || store->elements_type_matches(
                typeid(Numeric::int32).name()
            );
        }
        
        std::string message = "<nothing>";

        if(element_id != index_t(-1) && element_type != MESH_NONE) {
            message =
                   "x=" + String::to_string(p.x) +
                "\\ny=" + String::to_string(p.y) +
                "\\nz=" + String::to_string(p.z) ;                 
            message += "\\n" +
                mesh_grob()->subelements_type_to_name(element_type) +
                ": #" + String::to_string(element_id);
            if(with_value) {
                std::string value_str;
                if(is_bool) {
                    value_str = (value > 0.5) ? "true" : "false";
                } else if(is_integer) {
                    value_str = String::to_string(int(value));
                } else {
                    value_str = String::to_string(value);
                }
                if(dim > 1) {
                    attribute_name =
                        attribute_name + "["
                        + String::to_string(component) + "]";
                }
                message += ("\\n" + attribute_name + "=" + value_str);
                picked_ = true;
                value_ = value;
            }
        }
        set_tooltip(message);
    }

    void MeshGrobProbe::release(const RayPick& p_ndc) {
        geo_argused(p_ndc);
        reset_tooltip();

        if(picked_) {
            static const char* paint_tools[] = {
                "OGF::MeshGrobPaint",
                "OGF::MeshGrobPaintRect",
                nullptr
            };

            for(const char** p = paint_tools; *p != nullptr; ++p) {
                MeshGrobPaintTool* paint_tool =
                dynamic_cast<MeshGrobPaintTool*>(
                    tools_manager()->resolve_tool(*p)
                );
                geo_assert(paint_tool != nullptr);      
                paint_tool->set_value(value_);
                paint_tool->set_accumulate(false);
                paint_tool->set_autorange(false);
            }
        }
    }

/***************************************************************/

    MeshGrobRuler::MeshGrobRuler(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {
    }

    void MeshGrobRuler::grab(const RayPick& p_ndc) {
        latest_ndc_    = p_ndc.p_ndc;
        p_picked_ = pick(p_ndc, p_);
    }

    void MeshGrobRuler::drag(const RayPick& p_ndc) {

        if(length(p_ndc.p_ndc - latest_ndc_) <= 10.0/1024.0) {
            return ;
        }
        
        latest_ndc_ = p_ndc.p_ndc;
        
        vec3 q;
        bool q_picked = pick(p_ndc,q);
        std::string message;
        if(p_picked_ && q_picked) {
            message =
                "distance: " + String::to_string(length(q-p_));
        } else {
            message = "<nothing>";
        }
        set_tooltip(message);

        rendering_context()->overlay().clear();
        if(p_picked_ && q_picked) {
            rendering_context()->overlay().fillcircle(
                project_point(p_), 7.0,
                Color(1.0, 1.0, 1.0, 1.0)                
            );
            rendering_context()->overlay().fillcircle(
                project_point(q), 7.0,
                Color(1.0, 1.0, 1.0, 1.0)                
            );
            rendering_context()->overlay().segment(
                project_point(p_), project_point(q),
                Color(1.0, 1.0, 1.0, 1.0),
                3.0
            );
        }
    }

    void MeshGrobRuler::release(const RayPick& p_ndc) {
        geo_argused(p_ndc);
        reset_tooltip();
        rendering_context()->overlay().clear();
    }

    bool MeshGrobRuler::pick(const RayPick& p_ndc, vec3& p) {
        bool picked = false;
        MeshElementsFlags where[4] = {
            MESH_VERTICES, MESH_EDGES, MESH_FACETS, MESH_CELLS
        };
        for(index_t i=0; i<4; ++i) {
            picked = (MeshGrobTool::pick(p_ndc, where[i]) != index_t(-1));
            if(picked) {
                p = picked_point();
                return true;
            }
        }
        return false;
    }
    
/***************************************************************/

}
