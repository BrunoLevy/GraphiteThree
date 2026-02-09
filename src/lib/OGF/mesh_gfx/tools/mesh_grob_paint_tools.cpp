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

#include <OGF/mesh_gfx/tools/mesh_grob_paint_tools.h>
#include <OGF/mesh_gfx/shaders/mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/gom/reflection/meta.h>

#include <geogram/mesh/mesh_geometry.h>
#include <geogram/image/image_library.h>
#include <geogram/image/image_rasterizer.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/algorithm.h>

#include <stack>

namespace {
    using namespace OGF;

    /**
     * \brief Gets the visible attribute from the shader of a MeshGrob
     * \param[in] mesh_grob the MeshGrob
     * \param[out] where one of MESH_VERTICES, MESH_FACETS, MESH_CELLS
     * \param[out] attribute_name the base name of the attribute
     * \param[out] component the component index for a vector attribute
     *             (0 if it is a scalar attribute)
     */
    bool get_visible_attribute(
        MeshGrob* mesh_grob,
        MeshElementsFlags& where,
        std::string& attribute_name,
        index_t& component
    ) {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob->get_shader()
        );

        if(shd == nullptr) {
            return false;
        }

        std::string painting;
        shd->get_property("painting",painting);
        if(painting != "ATTRIBUTE") {
            return false;
        }

        std::string full_attribute_name;
        shd->get_property("attribute",full_attribute_name);
        if(!Mesh::parse_attribute_name(
               full_attribute_name,where,attribute_name,component)
          ) {
            return false;
        }

        return true;
    }

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
     * \param[in] value the value to be painted if op is PAINT_SET
     * \param[in] value2 the value to be painted if op is PAINT_RESET
     * \retval true if an attribute of the specified type was found
     * \retval false otherwise
     */
    template <class T> bool paint_attribute_generic(
        MeshGrob* mesh_grob, MeshElementsFlags where,
        const std::string& name, index_t component,
        index_t element_id, PaintOp op, double value, double value2
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
            attr[element_id*attr.dimension()+component] = T(value2);
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
     * \param[in] value the value to be painted if op is PAINT_SET
     * \param[in] value2 the value to be painted if op is PAINT_RESET
     * \retval true if an attribute could be painted
     * \retval false otherwise
     */
    bool paint_attribute(
        MeshGrob* mesh_grob, MeshElementsFlags where,
        const std::string& name, index_t component,
        index_t element_id, PaintOp op, double value, double value2
    ) {
        return paint_attribute_generic<double>(
                   mesh_grob, where, name, component, element_id,
		   op, value, value2
               ) ||
               paint_attribute_generic<float>(
                   mesh_grob, where, name, component, element_id,
		   op, value, value2
               ) ||
               paint_attribute_generic<Numeric::uint32>(
                   mesh_grob, where, name, component, element_id,
		   op, value, value2
               ) ||
               paint_attribute_generic<Numeric::int32>(
                   mesh_grob, where, name, component, element_id,
		   op, value, value2
               ) ||
               paint_attribute_generic<bool>(
                   mesh_grob, where, name, component, element_id,
		   op, value, std::max(value2, 0.0) // value <= 0 is false
               );
    }

    /**
     * \brief Calls a user-specified function for each facet in a connected
     *  component incident to a given facet
     * \param[in] mesh_grob the mesh
     * \param[in] seed_facet one of the facets of the connected component
     * \param[in] doit the function to be called for each facet of the
     *  connected component incident to \p seed_facet
     */
    void for_each_connected_facet(
        MeshGrob* mesh_grob, index_t seed_facet,
        std::function<bool(index_t)> doit
    ) {
        std::vector<bool> visited(mesh_grob->facets.nb(),false);
        std::stack<index_t> S;
        S.push(seed_facet);
        visited[seed_facet] = true;
        doit(seed_facet);
        while(!S.empty()) {
            index_t f = S.top();
            S.pop();
            for(index_t le=0; le<mesh_grob->facets.nb_vertices(f); ++le) {
                index_t g = mesh_grob->facets.adjacent(f,le);
                if(g != NO_INDEX && !visited[g]) {
                    if(doit(g)) {
                        S.push(g);
                    }
                    visited[g] = true;
                }
            }
        }
    }

    /**
     * \brief Calls a user-specified function for each cell in a connected
     *  component incident to a given cell
     * \param[in] mesh_grob the mesh
     * \param[in] seed_cell one of the cells of the connected component
     * \param[in] doit the function to be called for each cell of the
     *  connected component incident to \p seed_cell
     */

    void for_each_connected_cell(
        MeshGrob* mesh_grob, index_t seed_cell,
        std::function<bool(index_t)> doit
    ) {
        std::vector<bool> visited(mesh_grob->cells.nb(),false);
        std::stack<index_t> S;
        S.push(seed_cell);
        visited[seed_cell] = true;
        doit(seed_cell);
        while(!S.empty()) {
            index_t c = S.top();
            S.pop();
            for(index_t lf=0; lf<mesh_grob->cells.nb_facets(c); ++lf) {
                index_t d = mesh_grob->cells.adjacent(c,lf);
                if(d != NO_INDEX && !visited[d]) {
                    if(doit(d)) {
                        S.push(d);
                    }
                    visited[d] = true;
                }
            }
        }
    }

    /**
     * \brief Gets all the vertices of a connected component incident to
     *  a given facet
     * \param[in] mesh_grob the mesh
     * \param[in] seed_facet a facet of the connected component
     * \param[in,out] vertices the vertices of the connected component are
     *  appended here. Note that they may appear several times.
     */
    void get_connected_facet_vertices(
        MeshGrob* mesh_grob, index_t seed_facet,
        vector<index_t>& vertices
    ) {
        for_each_connected_facet(
            mesh_grob, seed_facet,
            [&](index_t f)->bool {
                for(index_t lv=0; lv<mesh_grob->facets.nb_vertices(f); ++lv) {
                    vertices.push_back(mesh_grob->facets.vertex(f,lv));
                }
                return true;
            }
        );
    }

    /**
     * \brief Gets all the vertices of a connected component incident to
     *  a given cell
     * \param[in] mesh_grob the mesh
     * \param[in] seed_cell a cell of the connected component
     * \param[in,out] vertices the vertices of the connected component are
     *  appended here. Note that they may appear several times.
     */
    void get_connected_cell_vertices(
        MeshGrob* mesh_grob, index_t seed_cell,
        vector<index_t>& vertices
    ) {
        for_each_connected_cell(
            mesh_grob, seed_cell,
            [&](index_t c)->bool {
                for(index_t lv=0; lv<mesh_grob->cells.nb_vertices(c); ++lv) {
                    vertices.push_back(mesh_grob->cells.vertex(c,lv));
                }
                return true;
            }
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

    /**
     * \brief called a function for each unique picked element in a
     *  picking image
     * \param[in,out] picking_image the image. It needs to be in RGBA format.
     *  It is modified by the function.
     * \param[in] mask an optional pointer to a mask (or nullptr for no mask).
     * \param[in] doit the function to be called for each unique picked element
     */
    void for_each_picked_element(
        Image* picking_image, Image* mask, std::function<void(index_t)> doit
    ) {
        geo_assert(picking_image->color_encoding() == Image::RGBA);
        geo_assert(picking_image->component_encoding() == Image::BYTE);

        // Apply mask to picking image
        if(mask != nullptr) {
            geo_assert(mask->color_encoding() == Image::GRAY);
            geo_assert(mask->component_encoding() == Image::BYTE);
            geo_assert(mask->width() == picking_image->width());
            geo_assert(mask->height() == picking_image->height());
            for(index_t y=0; y<picking_image->height(); ++y) {
                for(index_t x=0; x<picking_image->width(); ++x) {
                    if(*mask->pixel_base_byte_ptr(x,y) == 0) {
                        *picking_image->pixel_base_int32_ptr(x,y) =
                            Numeric::int32(-1);
                    }
                }
            }
        }

        // Get all the picked ids, by sorting the pixels of the image by
        // value then using std::unique
        Numeric::int32* begin = picking_image->pixel_base_int32_ptr(0,0);
        Numeric::int32* end =
	    begin + picking_image->width() * picking_image->height();
        std::sort(begin, end);
        end = std::unique(begin,end);
        for(Numeric::int32* p=begin; p!=end; ++p) {
            if(*p != -1) {
                doit(index_t(*p));
            }
        }
    }

    /**
     * \brief Tests whether a point belongs to a selection
     * \details The selection is a rectangle in device coordinates plus an
     *  optional mask image
     * \param[in] p the device coordinates of the point to be tested
     * \param[in] x0 , y0 , x1 , y1 the device coordinates rectangle
     * \param[in] mask the optional mask image. Needs to be in GRAYSCALE,
     *  8 bits per pixel, with same dimensions as rectangle.
     */
    bool point_is_selected(
        const vec2& p,
        index_t x0, index_t y0, index_t x1, index_t y1,
        Image* mask = nullptr
    ) {
        if(p.x < double(x0) || p.y < double(y0) ||
           p.x > double(x1) || p.y > double(y1)) {
            return false;
        }
        if(mask != nullptr) {
            if(*mask->pixel_base_byte_ptr(index_t(p.x)-x0,index_t(p.y)-y0)==0){
                return false;
            }
        }
        return true;
    }

}

namespace OGF {

    MeshGrobPaintTool::MeshGrobPaintTool(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {
       value_ = 1.0;
       value2_ = 0.0;
       accumulate_ = false;
       autorange_ = true;
       xray_mode_ = false;
       pick_vertices_only_ = true;
       picked_element_ = NO_INDEX;
       MeshGrobProbe* probe_tool = new MeshGrobProbe(parent);
       probe_tool->set_probed_as_paint(false);
       probe_tool->set_display_probed_on_release(false);
       probe_tool_ = probe_tool;
    }

    void MeshGrobPaintTool::reset() {
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        if(!get_visible_attribute(mesh_grob(),where,attribute_name,component)){
            Object_var I = mesh_grob()->query_interface("Selections");
            if(!I.is_null()) {
                I->invoke_method("show_vertices_selection");
            }
            where = MESH_VERTICES;
        }
        if(where == MESH_VERTICES) {
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

    bool MeshGrobPaintTool::get_painting_parameters(
        const RayPick& raypick,
        PaintOp& op,
        MeshElementsFlags& where,
        std::string& attribute_name,
        index_t& component
    ) {
        if(!get_visible_attribute(mesh_grob(),where,attribute_name,component)){
            return false;
        }
        op = PAINT_SET;
        if(accumulate_) {
            op = (raypick.button==MOUSE_BUTTON_LEFT) ? PAINT_INC : PAINT_DEC;
        } else {
            op = (raypick.button==MOUSE_BUTTON_LEFT) ? PAINT_SET : PAINT_RESET;
        }
        return true;
    }

    void MeshGrobPaintTool::paint(const RayPick& raypick) {

        PaintOp op = PAINT_SET;
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;

        if(!get_painting_parameters(raypick,op,where,attribute_name,component)){
            return;
        }

        index_t picked_element = pick(raypick,where);

        // Paint the picked element

        if(picked_element != NO_INDEX) {
            paint_attribute(
                mesh_grob(), where,
                attribute_name, component,
                picked_element, op, value_, value2_
            );
        } else if(where == MESH_VERTICES && !pick_vertices_only_) {

            // If painting vertices and no vertex was picked, try to
            // pick a facet or a cell, and paint its vertices.

            index_t f = pick_facet(raypick);
            if(f != NO_INDEX) {
                picked_element_ = f;
                for(index_t lv = 0;
                    lv<mesh_grob()->facets.nb_vertices(f); ++lv
                ) {
                    index_t v = mesh_grob()->facets.vertex(f,lv);
                    paint_attribute(
                        mesh_grob(), where,
                        attribute_name, component,
                        v, op, value_, value2_
                    );
                }
            } else {
                index_t c = pick_cell(raypick);
                picked_element_ = c;
                if(c != NO_INDEX) {
                    for(index_t lv = 0;
                        lv<mesh_grob()->cells.nb_vertices(c); ++lv
                    ) {
                        index_t v = mesh_grob()->cells.vertex(c,lv);
                        paint_attribute(
                            mesh_grob(), where,
                            attribute_name, component,
                            v, op, value_, value2_
                        );
                    }
                }
            }
        }

        if(picked_element != picked_element_) {
            if(autorange_) {
                if(mesh_grob()->get_shader() != nullptr) {
                    mesh_grob()->get_shader()->invoke_method("autorange");
                }
            }
            mesh_grob()->update();
            picked_element_ = picked_element;
        }
    }

    void MeshGrobPaintTool::set_value(double value) {
        // set property for all MeshGrobPaintTools.
        vector<MeshGrobPaintTool*> tools;
        get_paint_tools(tools_manager(), tools);
        for(MeshGrobPaintTool* tool: tools) {
            tool->set_value_for_this_tool(value);
        }
    }

    void MeshGrobPaintTool::set_value2(double value) {
        // set property for all MeshGrobPaintTools.
        vector<MeshGrobPaintTool*> tools;
        get_paint_tools(tools_manager(), tools);
        for(MeshGrobPaintTool* tool: tools) {
            tool->set_value2_for_this_tool(value);
        }
    }

    void MeshGrobPaintTool::set_accumulate(bool value) {
        // set property for all MeshGrobPaintTools.
        vector<MeshGrobPaintTool*> tools;
        get_paint_tools(tools_manager(), tools);
        for(MeshGrobPaintTool* tool: tools) {
            tool->set_accumulate_for_this_tool(value);
        }
    }

    void MeshGrobPaintTool::set_autorange(bool value) {
        // set property for all MeshGrobPaintTools.
        vector<MeshGrobPaintTool*> tools;
        get_paint_tools(tools_manager(), tools);
        for(MeshGrobPaintTool* tool: tools) {
            tool->set_autorange_for_this_tool(value);
        }
    }

    void MeshGrobPaintTool::set_pick_vertices_only(bool value) {
        // set property for all MeshGrobPaintTools.
        vector<MeshGrobPaintTool*> tools;
        get_paint_tools(tools_manager(), tools);
        for(MeshGrobPaintTool* tool: tools) {
            tool->set_pick_vertices_for_this_tool(value);
        }

        // if only vertices can be picked, then display them
        // (else hide them).
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

    void MeshGrobPaintTool::set_xray_mode(bool value) {
        // set property for all MeshGrobPaintTools.
        vector<MeshGrobPaintTool*> tools;
        get_paint_tools(tools_manager(), tools);
        for(MeshGrobPaintTool* tool: tools) {
            tool->set_xray_mode_for_this_tool(value);
        }
    }

    void MeshGrobPaintTool::get_paint_tools(
        ToolsManager* manager, vector<MeshGrobPaintTool*>& paint_tools
    ) {
        paint_tools.clear();

        // Get OGF::MeshGrobPaintTool meta type
        MetaType* mesh_grob_paint_tool_type =
            Meta::instance()->resolve_meta_type("OGF::MeshGrobPaintTool");
        geo_assert(mesh_grob_paint_tool_type != nullptr);

        // Iterate on all meta types known in the system, and
        // pick the ones that derive from OGF::MeshGrobPaintTool
        std::vector<MetaType*> all_types;
        Meta::instance()->list_types(all_types);
        for(MetaType* mtype : all_types) {
            if(mtype->is_subtype_of(mesh_grob_paint_tool_type)) {
                // Find (or create) the corresponding tool
                // in the tools manager
                MeshGrobPaintTool* paint_tool =
                    dynamic_cast<MeshGrobPaintTool*>(
                        manager->resolve_tool(mtype->name())
                    );
                geo_assert(paint_tool != nullptr);
                paint_tools.push_back(paint_tool);
            }
        }
    }

    void MeshGrobPaintTool::grab(const RayPick& p_ndc) {
	if(p_ndc.button == 2) {
	    probe_tool_->grab(p_ndc);
 	} else {
	    MeshGrobTool::grab(p_ndc);
	}
    }

    void MeshGrobPaintTool::drag(const RayPick& p_ndc) {
	if(p_ndc.button == 2) {
	    probe_tool_->drag(p_ndc);
	} else {
	    MeshGrobTool::drag(p_ndc);
	}
    }

    void MeshGrobPaintTool::release(const RayPick& p_ndc) {
	if(p_ndc.button == 2) {
	    probe_tool_->release(p_ndc);
	} else {
	    MeshGrobTool::release(p_ndc);
	}
    }

    /**********************************************************************/

    MeshGrobPaint::MeshGrobPaint(
        ToolsManager* parent
    ) : MeshGrobPaintRect(parent) {
        stroke_mode_ = true;
        width_ = 5;
    }

    void MeshGrobPaint::grab(const RayPick& raypick) {
	if(raypick.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::grab(raypick);
	    return;
	}
        MeshGrobPaintTool::grab(raypick);
        latest_ndc_ = raypick.p_ndc;
        if(stroke_mode_) {
            stroke_.push_back(ndc_to_dc(raypick.p_ndc));
        } else {
            paint(raypick);
        }
    }

    void MeshGrobPaint::drag(const RayPick& raypick) {
	if(raypick.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::drag(raypick);
	    return;
	}
        if(length(raypick.p_ndc - latest_ndc_) <= 10.0/1024.0) {
            return ;
        }
        latest_ndc_ = raypick.p_ndc;

        // In stroke mode, draw the stroke in the overlay
        if(stroke_mode_) {
            stroke_.push_back(ndc_to_dc(raypick.p_ndc));

            rendering_context()->overlay().clear();
            for(index_t i=0; i<stroke_.size(); ++i) {
                rendering_context()->overlay().fillcircle(
                    stroke_[i],double(width_),Color(1.0, 1.0, 1.0, 1.0)
                );
            }
            for_each_stroke_quad(
                [&](vec2 q1, vec2 q2, vec2 q3, vec2 q4) {
                    rendering_context()->overlay().fillquad(
                        q1,q2,q3,q4,Color(1.0, 1.0, 1.0, 1.0)
                    );
                }
            );
        } else {
            // If not in stroke mode, directly paint the picked element
            paint(raypick);
        }
    }

    void MeshGrobPaint::release(const RayPick& raypick) {
	if(raypick.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::release(raypick);
	    return;
	}

        if(stroke_mode_ && stroke_.size() != 0) {

            // Get the bounding box of the stroke
            int x0 =  65535;
            int y0 =  65535;
            int x1 = -65535;
            int y1 = -65535;
            for_each_stroke_quad(
                [&](vec2 q1, vec2 q2, vec2 q3, vec2 q4) {
                    vec2 t[4] = {q1,q2,q3,q4};
                    for(index_t i=0; i<4; ++i) {
                        x0 = std::min(x0, int(t[i].x));
                        y0 = std::min(y0, int(t[i].y));
                        x1 = std::max(x1, int(t[i].x));
                        y1 = std::max(y1, int(t[i].y));
                    }
                }
            );

            // Enlarge the bbox to take into account the stroke width
            x0 -= int(width_);
            y0 -= int(width_);
            x1 += int(width_);
            y1 += int(width_);

            // Clip bounding box
            x0 = std::max(x0,0);
            y0 = std::max(y0,0);
            x1 = std::min(x1,int(rendering_context()->get_width() -1));
            y1 = std::min(y1,int(rendering_context()->get_height()-1));

            if(x1 > x0 && y1 > y0) {

                // Generate mask

                Image_var mask = new Image(
                    Image::GRAY, Image::BYTE,
                    index_t(x1-x0+1),
                    index_t(y1-y0+1)
                );
                ImageRasterizer rasterizer(mask);

                Color white(1.0, 1.0, 1.0, 1.0);

                if(stroke_.size() != 0) {
                    double r = double(width_)/double(x1-x0+1);
                    for(index_t i=0; i<stroke_.size(); ++i) {
                        vec2 c = stroke_[i];
                        c -= vec2(double(x0), double(y0));
                        c.x /= double(x1-x0+1);
                        c.y /= double(y1-y0+1);
                        rasterizer.fillcircle(c,r,white);
                    }
                }

                for_each_stroke_quad(
                    [&](vec2 q1, vec2 q2, vec2 q3, vec2 q4) {
                        vec2 t[4] = {q1,q2,q3,q4};
                        for(index_t i=0; i<4; ++i) {
                            t[i] -= vec2(double(x0), double(y0));
                            t[i].x /= double(x1-x0+1);
                            t[i].y /= double(y1-y0+1);
                            t[i].x = std::max(t[i].x,0.0);
                            t[i].y = std::max(t[i].y,0.0);
                            t[i].x = std::min(t[i].x,1.0);
                            t[i].y = std::min(t[i].y,1.0);
                        }
                        rasterizer.triangle(t[0],white,t[1],white,t[2],white);
                        rasterizer.triangle(t[0],white,t[2],white,t[3],white);
                    }
                );

                paint_rect(
                    raypick, index_t(x0), index_t(y0), index_t(x1), index_t(y1),
                    mask
                );

            } else {
                paint(raypick);
            }

            stroke_.clear();
            rendering_context()->overlay().clear();
        } else {
            paint(raypick);
        }

        if(autorange_) {
            if(mesh_grob()->get_shader() != nullptr) {
                mesh_grob()->get_shader()->invoke_method("autorange");
            }
        }
    }

    void MeshGrobPaint::for_each_stroke_quad(
        std::function<void(vec2, vec2, vec2, vec2)> doit
    ) {
        for(index_t i=0; i+1<stroke_.size(); ++i) {
            vec2 p1 = stroke_[i];
            vec2 p2 = stroke_[i+1];
            vec2 n1 = p2-p1;
            vec2 n2 = p2-p1;
            if(i > 0) {
                n1 += p1 - stroke_[i-1];
            }
            if(i+2<stroke_.size()) {
                n2 += stroke_[i+2] - p2;
            }

            n1 = normalize(vec2(n1.y, -n1.x));
            n2 = normalize(vec2(n2.y, -n2.x));

            double width = double(width_);
            vec2 q1 = p1-width*n1;
            vec2 q2 = p1+width*n1;
            vec2 q3 = p2-width*n2;
            vec2 q4 = p2+width*n2;

            doit(q1,q2,q4,q3);
        }
    }

    /**********************************************************************/

    MeshGrobPaintRect::MeshGrobPaintRect(
        ToolsManager* parent
    ) : MeshGrobPaintTool(parent) {
    }

    void MeshGrobPaintRect::grab(const RayPick& p_ndc) {
	if(p_ndc.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::grab(p_ndc);
	    return;
	}

        MeshGrobPaintTool::grab(p_ndc);
        p_ = ndc_to_dc(p_ndc.p_ndc);
    }

    void MeshGrobPaintRect::drag(const RayPick& p_ndc) {
	if(p_ndc.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::drag(p_ndc);
	    return;
	}

        // Draw selection rectangle in overlay
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
	if(raypick.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::release(raypick);
	    return;
	}

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
        index_t x0, index_t y0, index_t x1, index_t y1,
        Image* mask
    ) {
        if(x1 == x0 || y1 == y0) {
            return;
        }

        if(mask != nullptr) {
            geo_assert(mask->color_encoding() == Image::GRAY);
            geo_assert(mask->component_encoding() == Image::BYTE);
            geo_assert(mask->width() == x1-x0+1);
            geo_assert(mask->height() == y1-y0+1);
        }

        PaintOp op = PAINT_SET;
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        if(
            !get_painting_parameters(
                raypick, op, where, attribute_name, component
            )
        ) {
            return;
        }

        // Pick the elements, and copy the selected rect in an image
        index_t width  = x1-x0+1;
        index_t height = y1-y0+1;
        Image_var picking_image = new Image;
        // We need 32-bit pixel values (default is 24-bit)
        picking_image->initialize(Image::RGBA, Image::BYTE, width, height);


        // In xray mode, test for each element whether its center falls in the
        // selection.
        if(xray_mode_) {
            switch(where) {
            case MESH_VERTICES: {
                for(index_t v: mesh_grob()->vertices) {
                    vec2 p = project_point(
                        vec3(mesh_grob()->vertices.point_ptr(v))
                    );
                    if(point_is_selected(p,x0,y0,x1,y1,mask)) {
                        paint_attribute(
                            mesh_grob(), where,
                            attribute_name, component,
                            v, op, value_, value2_
                        );
                    }
                }
            } break;
            case MESH_FACETS: {
                for(index_t f: mesh_grob()->facets) {
                    vec2 p = project_point(
                        Geom::mesh_facet_center(*mesh_grob(),f)
                    );
                    if(point_is_selected(p,x0,y0,x1,y1,mask)) {
                        paint_attribute(
                            mesh_grob(), where,
                            attribute_name, component,
                            f, op, value_, value2_
                        );
                    }
                }
            } break;
            case MESH_CELLS: {
                for(index_t c: mesh_grob()->cells) {
                    vec2 p = project_point(
                        Geom::mesh_cell_center(*mesh_grob(),c)
                    );
                    if(point_is_selected(p,x0,y0,x1,y1,mask)) {
                        paint_attribute(
                            mesh_grob(), where,
                            attribute_name, component,
                            c, op, value_, value2_
                        );
                    }
                }
            } break;
	    case MESH_NONE:
	    case MESH_EDGES:
	    case MESH_ALL_ELEMENTS:
	    case MESH_FACET_CORNERS:
	    case MESH_CELL_CORNERS:
	    case MESH_CELL_FACETS:
	    case MESH_ALL_SUBELEMENTS:
		break;
            }
        } else {

            // In standard mode, get picking image, apply the (optional) mask
            // and find all the picked elements

            pick(raypick, where, picking_image, x0, y0, width, height);

            for_each_picked_element(
                picking_image, mask,
                [&](index_t picked_element) {
                    paint_attribute(
                        mesh_grob(), where,
                        attribute_name, component,
                        picked_element, op, value_, value2_
                    );
                }
            );

            // If painting vertices and no vertex was picked, try to
            // pick a facet or a cell, and paint its vertices.

            if(!pick_vertices_only_ && where == MESH_VERTICES) {
                pick(raypick,MESH_FACETS, picking_image, x0, y0, width, height);
                for_each_picked_element(
                    picking_image, mask,
                    [&](index_t f) {
                        for(index_t lv = 0;
                            lv<mesh_grob()->facets.nb_vertices(f); ++lv
                           ) {
                            index_t v = mesh_grob()->facets.vertex(f,lv);
                            paint_attribute(
                                mesh_grob(), where,
                                attribute_name, component,
                                v, op, value_, value2_
                            );
                        }
                    }
                );

                pick(raypick, MESH_CELLS, picking_image, x0, y0, width, height);
                for_each_picked_element(
                    picking_image, mask,
                    [&](index_t c) {
                        for(index_t lv = 0;
                            lv<mesh_grob()->cells.nb_vertices(c); ++lv
                        ) {
                            index_t v = mesh_grob()->cells.vertex(c,lv);
                            paint_attribute(
                                mesh_grob(), where,
                                attribute_name, component,
                                v, op, value_, value2_
                            );
                        }
                    }
                );
            }
        }
        if(autorange_) {
            if(mesh_grob()->get_shader() != nullptr) {
                mesh_grob()->get_shader()->invoke_method("autorange");
            }
        }
    }

    /***************************************************************/

    MeshGrobPaintFreeform::MeshGrobPaintFreeform(
        ToolsManager* parent
    ) : MeshGrobPaintRect(parent) {
    }

    void MeshGrobPaintFreeform::grab(const RayPick& raypick) {
	if(raypick.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::grab(raypick);
	    return;
	}

        MeshGrobPaintTool::grab(raypick);
        selection_.push_back(ndc_to_dc(raypick.p_ndc));
    }

    void MeshGrobPaintFreeform::drag(const RayPick& raypick) {
	if(raypick.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::drag(raypick);
	    return;
	}

        if(length(raypick.p_ndc - latest_ndc_) <= 10.0/1024.0) {
            return ;
        }
        latest_ndc_ = raypick.p_ndc;
        selection_.push_back(ndc_to_dc(raypick.p_ndc));
        rendering_context()->overlay().clear();
        for(index_t i=0; i<selection_.size(); ++i) {
            index_t j = (i+1)%selection_.size();
            rendering_context()->overlay().segment(
                selection_[i], selection_[j], Color(1.0, 1.0, 1.0, 1.0), 2.0
            );
        }
    }

    void MeshGrobPaintFreeform::release(const RayPick& raypick) {
	if(raypick.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::release(raypick);
	    return;
	}

        index_t x0,y0,width,height;
        {
            double minx =  Numeric::max_float64();
            double miny =  Numeric::max_float64();
            double maxx = -Numeric::max_float64();
            double maxy = -Numeric::max_float64();
            for(index_t i=0; i<selection_.size(); ++i) {
                minx = std::min(minx, selection_[i].x);
                miny = std::min(miny, selection_[i].y);
                maxx = std::max(maxx, selection_[i].x);
                maxy = std::max(maxy, selection_[i].y);
            }
            x0 = index_t(minx);
            y0 = index_t(miny);
            width = index_t(maxx-minx)+1;
            height = index_t(maxy-miny)+1;
        }

        Image_var mask = new Image(Image::GRAY,Image::BYTE, width, height);
        ImageRasterizer rasterizer(mask);
        Color black(0.0,0.0,0.0,1.0);
        Color white(1.0,1.0,1.0,1.0);

        for(index_t i=0; i<selection_.size(); ++i) {
            index_t j = (i+1)%selection_.size();
            vec2 p1 = selection_[i];
            vec2 p2 = selection_[j];
            p1 -= vec2(double(x0),double(y0));
            p2 -= vec2(double(x0),double(y0));
            p1.x /= double(width);
            p1.y /= double(height);
            p2.x /= double(width);
            p2.y /= double(height);
            rasterizer.segment(p1, p2, white);
        }

        // Flood-fill from all pixel borders, so that all
        // pixels outside the selection will be white
        // (we will invert right after)
        for(int x=0; x<int(mask->width()); ++x) {
            rasterizer.flood_fill(x,0,white);
            rasterizer.flood_fill(x,int(height-1),white);
        }
        for(int y=0; y<int(mask->height()); ++y) {
            rasterizer.flood_fill(0,y,white);
            rasterizer.flood_fill(int(width-1),y,white);
        }

        // Invert colors (more logical to have selection in
        // white and background in black)
        for(int y=0; y<int(height); ++y) {
            for(int x=0; x<int(width); ++x) {
                if(rasterizer.pixel_is_black(x,y)) {
                    rasterizer.set_pixel(x,y,white);
                } else {
                    rasterizer.set_pixel(x,y,black);
                }
            }
        }

        paint_rect(raypick, x0, y0, x0+width-1, y0+height-1, mask);

        selection_.clear();
        rendering_context()->overlay().clear();
    }

    /***************************************************************/

    MeshGrobPaintConnected::MeshGrobPaintConnected(
        ToolsManager* parent
    ) : MeshGrobPaintTool(parent) {
        fill_same_value_ = true;
        tolerance_ = 0.0;
    }

    void MeshGrobPaintConnected::grab(const RayPick& raypick) {
	if(raypick.button == 2) {
	    // Button 2 is for probe, handled in baseclass
	    MeshGrobPaintTool::grab(raypick);
	    return;
	}

        MeshGrobPaintTool::grab(raypick);
        PaintOp op = PAINT_SET;
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;

        if(!get_painting_parameters(raypick,op,where,attribute_name,component)){
            return;
        }

        index_t picked_element = pick(raypick, where);
        if(picked_element == NO_INDEX) {
            return;
        }

        double picked_value;
        bool with_value = probe_attribute(
            mesh_grob(), where,
            attribute_name, component,
            picked_element, picked_value
        );

        if(fill_same_value_ && !with_value) {
            return;
        }

        if(where != MESH_VERTICES && picked_element != NO_INDEX) {
            switch(where) {
            case MESH_FACETS: {
                for_each_connected_facet(
                    mesh_grob(), picked_element,
                    [&](index_t f)->bool {
                        double value;
                        probe_attribute(
                            mesh_grob(), where, attribute_name, component,
                            f, value
                        );
                        if(!test(picked_value, value)) {
                            return false;
                        }
                        paint_attribute(
                            mesh_grob(), where,
			    attribute_name, component, f,
			    op, value_, value2_
                        );
                        return true;
                    }
                );
            } break;
            case MESH_CELLS: {
                for_each_connected_cell(
                    mesh_grob(), picked_element,
                    [&](index_t c)->bool {
                        double value;
                        probe_attribute(
                            mesh_grob(), where, attribute_name, component,
                            c, value
                        );
                        if(!test(picked_value, value)) {
                            return false;
                        }
                        paint_attribute(
                          mesh_grob(), where, attribute_name, component, c,
			  op, value_, value2_
                        );
                        return true;
                    }
                );
            } break;
	    case MESH_NONE:
	    case MESH_VERTICES:
	    case MESH_EDGES:
	    case MESH_ALL_ELEMENTS:
	    case MESH_FACET_CORNERS:
	    case MESH_CELL_CORNERS:
	    case MESH_CELL_FACETS:
	    case MESH_ALL_SUBELEMENTS:
		break;
            }
        } else if(where == MESH_VERTICES) {
            vector<index_t> vertices;
            picked_element = pick_facet(raypick);
            if(picked_element != NO_INDEX) {
                get_connected_facet_vertices(
                    mesh_grob(), picked_element, vertices
                );
                // if a cell facet was picked, propagate also to the
                // volume connected component (obtained by finding a cell
                // incident to one of the vertices of the picked facet)
                index_t v = mesh_grob()->facets.vertex(picked_element,0);
                for(index_t c: mesh_grob()->cells) {
                    bool found = false;
                    for(index_t lv=0;lv<mesh_grob()->cells.nb_vertices(c);++lv){
                        if(mesh_grob()->cells.vertex(c,lv) == v) {
                            found = true;
                            break;
                        }
                    }
                    if(found) {
                        get_connected_cell_vertices(mesh_grob(),c,vertices);
                        break;
                    }
                }
            }
            picked_element = pick_cell(raypick);
            if(picked_element != NO_INDEX) {
                get_connected_cell_vertices(
                    mesh_grob(), picked_element, vertices
                );
            }
            sort_unique(vertices);
            for(index_t v: vertices) {
                paint_attribute(
                    mesh_grob(), MESH_VERTICES,
                    attribute_name, component, v, op, value_, value2_
                );
            }
        }
        if(autorange_) {
            if(mesh_grob()->get_shader() != nullptr) {
                mesh_grob()->get_shader()->invoke_method("autorange");
            }
        }
    }

    /***************************************************************/

    MeshGrobProbe::MeshGrobProbe(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {
        picked_ = false;
        grabbed_ = false;
	set_probed_as_paint_ = true;
	display_probed_on_release_ = true;
    }

    void MeshGrobProbe::grab(const RayPick& p_ndc) {
        latest_ndc_ = p_ndc.p_ndc;
        probe(p_ndc);
        grabbed_ = true;
    }

    void MeshGrobProbe::drag(const RayPick& p_ndc) {
        if(!grabbed_) {
            return;
        }
        if(length(p_ndc.p_ndc - latest_ndc_) <= 10.0/1024.0) {
            return ;
        }
        latest_ndc_ = p_ndc.p_ndc;
        probe(p_ndc);
    }

    void MeshGrobProbe::release(const RayPick& p_ndc) {
        geo_argused(p_ndc);
        reset_tooltip();
        if(!grabbed_) {
            return;
        }
        if(picked_ && set_probed_as_paint_) {
            vector<MeshGrobPaintTool*> tools;
            MeshGrobPaintTool::get_paint_tools(tools_manager(), tools);
            for(MeshGrobPaintTool* tool: tools) {
                tool->set_value_for_this_tool(value_);
                tool->set_accumulate_for_this_tool(false);
                tool->set_autorange_for_this_tool(false);
            }
        }
	if(display_probed_on_release_) {
	    std::vector<std::string> lines;
	    String::split_string(message_,"\\n",lines);
	    for(const std::string& line : lines) {
		Logger::out("Probe") << line << std::endl;
	    }
	    Logger::out("") << std::endl;
	}
        grabbed_ = false;
    }

    void MeshGrobProbe::probe(const RayPick& p_ndc) {
        picked_ = false;

        MeshElementsFlags attribute_element_type;
        std::string attribute_name;
        index_t component;
        bool with_attributes = get_visible_attribute(
            mesh_grob(), attribute_element_type, attribute_name, component
        );

        double value = 0.0;
        bool with_value = false;
        index_t element_id = NO_INDEX;
        MeshElementsFlags element_type = MESH_NONE;
        vec3 p;

        static MeshElementsFlags element_types[4] = {
            MESH_VERTICES, MESH_EDGES, MESH_FACETS, MESH_CELLS
        };

        for(index_t i=0; i<4; ++i) {
            if(element_id == NO_INDEX) {
                element_id = pick(p_ndc, element_types[i]);
                if(element_id != NO_INDEX) {
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
            !with_value     &&
            attribute_element_type == MESH_VERTICES &&
            element_id != NO_INDEX
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

        message_ = "<nothing>";

        if(element_id != NO_INDEX && element_type != MESH_NONE) {
            message_ =
                   "x=" + String::to_string(p.x) +
                "\\ny=" + String::to_string(p.y) +
                "\\nz=" + String::to_string(p.z) ;
            message_ += "\\n" +
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
                message_ += ("\\n" + attribute_name + "=" + value_str);
                picked_ = true;
                value_ = value;
            }
        }
        set_tooltip(message_);
    }


    /***************************************************************/

    MeshGrobRuler::MeshGrobRuler(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {
    }

    void MeshGrobRuler::grab(const RayPick& rp) {
        latest_ndc_    = rp.p_ndc;
        p_picked_ = pick(rp, p_);
    }

    void MeshGrobRuler::drag(const RayPick& rp) {

        if(length(rp.p_ndc - latest_ndc_) <= 10.0/1024.0) {
            return ;
        }

        latest_ndc_ = rp.p_ndc;

        vec3 q;
        bool q_picked = pick(rp,q);
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
            picked = (MeshGrobTool::pick(p_ndc, where[i]) != NO_INDEX);
            if(picked) {
                p = picked_point();
                return true;
            }
        }
        return false;
    }

/***************************************************************/

}
