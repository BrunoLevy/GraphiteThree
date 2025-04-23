
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

#include <OGF/mesh_gfx/tools/mesh_grob_component_tools.h>
#include <geogram/mesh/mesh_geometry.h>
#include <stack>

namespace {
    using namespace OGF;

    static void mark_facet_vertices(
        const Mesh& M, index_t f, vector<bool>& v_is_marked
    ) {
        for(index_t lv=0; lv<M.facets.nb_vertices(f); ++lv) {
            v_is_marked[M.facets.vertex(f,lv)] = true;
        }
    }

    static void mark_cell_vertices(
        const Mesh& M, index_t c, vector<bool>& v_is_marked
    ) {
        for(index_t lv=0; lv<M.cells.nb_vertices(c); ++lv) {
            v_is_marked[M.cells.vertex(c,lv)] = true;
        }
    }

    static bool all_facet_vertices_are_marked(
        const Mesh& M, index_t f, vector<bool>& v_is_marked
    ) {
        for(index_t lv=0; lv<M.facets.nb_vertices(f); ++lv) {
            if(!v_is_marked[M.facets.vertex(f,lv)]) {
                return false;
            }
        }
        return true;
    }

    static bool all_edge_vertices_are_marked(
        const Mesh& M, index_t e, vector<bool>& v_is_marked
    ) {
        return
            v_is_marked[M.edges.vertex(e,0)] &&
            v_is_marked[M.edges.vertex(e,1)] ;
    }

    static bool all_cell_vertices_are_marked(
        const Mesh& M, index_t c, vector<bool>& v_is_marked
    ) {
        for(index_t lv=0; lv<M.cells.nb_vertices(c); ++lv) {
            if(!v_is_marked[M.cells.vertex(c,lv)]) {
                return false;
            }
        }
        return true;
    }

    void get_connected_component_from_facet(
        const Mesh& M, index_t f, vector<bool>& v_is_marked
    ) {
        vector<bool> f_is_marked(M.facets.nb(), false);
        std::stack<index_t> S;
        f_is_marked[f] = true;
        mark_facet_vertices(M,f,v_is_marked);
        S.push(f);
        while(!S.empty()) {
            f = S.top();
            S.pop();
            for(index_t le=0; le<M.facets.nb_vertices(f); ++le) {
                index_t adj = M.facets.adjacent(f,le);
                if(adj != NO_FACET && !f_is_marked[adj]) {
                    f_is_marked[adj] = true;
                    mark_facet_vertices(M,adj,v_is_marked);
                    S.push(adj);
                }
            }
        }
    }

    void propagate_connected_component_to_facets(
        const Mesh& M, vector<bool>& v_is_marked
    ) {
        vector<bool> f_is_marked(M.facets.nb(), false);
        std::stack<index_t> S;
        for(index_t f: M.facets) {
            for(index_t lv=0; lv<M.facets.nb_vertices(f); ++lv) {
                index_t v = M.facets.vertex(f,lv);
                if(v_is_marked[v]) {
                    f_is_marked[f] = true;
                    mark_facet_vertices(M,f,v_is_marked);
                    S.push(f);
                    break;
                }
            }
        }
        while(!S.empty()) {
            index_t f = S.top();
            S.pop();
            for(index_t le=0; le<M.facets.nb_vertices(f); ++le) {
                index_t adj = M.facets.adjacent(f,le);
                if(adj != NO_FACET && !f_is_marked[adj]) {
                    f_is_marked[adj] = true;
                    mark_facet_vertices(M,adj,v_is_marked);
                    S.push(adj);
                }
            }
        }
    }


    void get_connected_component_from_cell(
        const Mesh& M, index_t c, vector<bool>& v_is_marked
    ) {
        vector<bool> c_is_marked(M.cells.nb(), false);
        std::stack<index_t> S;
        c_is_marked[c] = true;
        mark_cell_vertices(M,c,v_is_marked);
        S.push(c);
        while(!S.empty()) {
            c = S.top();
            S.pop();
            for(index_t lf=0; lf<M.cells.nb_facets(c); ++lf) {
                index_t adj = M.cells.adjacent(c,lf);
                if(adj != NO_CELL && !c_is_marked[adj]) {
                    c_is_marked[adj] = true;
                    mark_cell_vertices(M,adj,v_is_marked);
                    S.push(adj);
                }
            }
        }
    }

    void propagate_connected_component_to_cells(
        const Mesh& M, vector<bool>& v_is_marked
    ) {
        vector<bool> c_is_marked(M.cells.nb(), false);
        std::stack<index_t> S;

        for(index_t c: M.cells) {
            for(index_t lv=0; lv<M.cells.nb_vertices(c); ++lv) {
                index_t v = M.cells.vertex(c,lv);
                if(v_is_marked[v]) {
                    c_is_marked[c] = true;
                    mark_cell_vertices(M,c,v_is_marked);
                    S.push(c);
                    break;
                }
            }
        }
        while(!S.empty()) {
            index_t c = S.top();
            S.pop();
            for(index_t lf=0; lf<M.cells.nb_facets(c); ++lf) {
                index_t adj = M.cells.adjacent(c,lf);
                if(adj != NO_CELL && !c_is_marked[adj]) {
                    c_is_marked[adj] = true;
                    mark_cell_vertices(M,adj,v_is_marked);
                    S.push(adj);
                }
            }
        }
    }


    bool pick_component(
        MeshGrobTool* tool, const RayPick& rp, vector<bool>& v_is_picked
    ) {
        bool result = false;

        MeshGrob& mesh_grob = *tool->mesh_grob();
        v_is_picked.assign(mesh_grob.vertices.nb(), false);
        index_t picked_cell = NO_CELL;

        if(mesh_grob.cells.nb() != 0) {
            picked_cell = tool->pick_cell(rp);
        }
        if(picked_cell != NO_CELL) {
            result = true;
            get_connected_component_from_cell(
                mesh_grob, picked_cell, v_is_picked
            );
            propagate_connected_component_to_facets(
                mesh_grob, v_is_picked
            );
        } else {
            index_t picked_facet = tool->pick_facet(rp);
            if(picked_facet != NO_FACET) {
                result = true;
                get_connected_component_from_facet(
                    mesh_grob, picked_facet, v_is_picked
                );
                propagate_connected_component_to_cells(
                    mesh_grob, v_is_picked
                );
            }
        }
        return result;
    }

}

namespace OGF {

    void MeshGrobRemoveComponent::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        vector<bool> v_is_picked;
        if(pick_component(this, p_ndc, v_is_picked)) {
            if(mesh_grob()->facets.nb() != 0) {
                vector<index_t> to_remove(mesh_grob()->facets.nb(),0);
                for(index_t f: mesh_grob()->facets) {
                    bool remove_f =
                        all_facet_vertices_are_marked(
                            *mesh_grob(), f, v_is_picked
                        );
                    if(invert_selection_) {
                        remove_f = !remove_f;
                    }
                    if(remove_f) {
                        to_remove[f] = 1;
                    }
                }
                mesh_grob()->facets.delete_elements(to_remove);
            }
            if(mesh_grob()->edges.nb() != 0) {
                vector<index_t> to_remove(mesh_grob()->edges.nb(),0);
                for(index_t e: mesh_grob()->edges) {
                    bool remove_e =
                        all_edge_vertices_are_marked(
                            *mesh_grob(), e, v_is_picked
                        );
                    if(invert_selection_) {
                        remove_e = !remove_e;
                    }
                    if(remove_e) {
                        to_remove[e] = 1;
                    }
                }
                mesh_grob()->edges.delete_elements(to_remove);
            }
            if(mesh_grob()->cells.nb() != 0) {
                vector<index_t> to_remove(mesh_grob()->cells.nb(),0);
                for(index_t c: mesh_grob()->cells) {
                    bool remove_c =
                        all_cell_vertices_are_marked(
                            *mesh_grob(), c, v_is_picked
                        );
                    if(invert_selection_) {
                        remove_c = !remove_c;
                    }
                    if(remove_c) {
                        to_remove[c] = 1;
                    }
                }
                mesh_grob()->cells.delete_elements(to_remove);
            }
            mesh_grob()->vertices.remove_isolated();
            mesh_grob()->update();
        }
    }

/**********************************************************/

    void MeshGrobCopyComponent::pick_subset(
        MeshGrobTransformSubset* tool, const RayPick& rp
    ) {
        first_new_vertex_ = mesh_grob()->vertices.nb();
        vector<bool> v_is_picked;
        pick_component(tool, rp, v_is_picked);

        vector<index_t> v_to_new(mesh_grob()->vertices.nb(), NO_VERTEX);
        index_t nb_new_vertices = 0;
        for(index_t v=0; v<v_is_picked.size(); ++v) {
            if(v_is_picked[v]) {
                v_to_new[v] = first_new_vertex_ + nb_new_vertices;
                ++nb_new_vertices;
            }
        }

        // Copy the vertices
        mesh_grob()->vertices.create_vertices(nb_new_vertices);
        for(index_t v=0; v<first_new_vertex_; ++v) {
            if(v_is_picked[v]) {
                double* from = mesh_grob()->vertices.point_ptr(v);
                double* to = mesh_grob()->vertices.point_ptr(v_to_new[v]);
                for(index_t c=0; c<mesh_grob()->vertices.dimension(); ++c) {
                    to[c] = from[c];
                }
            }
        }

        // Copy the facets
        if(mesh_grob()->facets.nb() != 0) {
            index_t first_new_f = mesh_grob()->facets.nb();
            vector<index_t> f_to_new(mesh_grob()->facets.nb(), NO_FACET);
            index_t nb_new_f = 0;
            for(index_t f: mesh_grob()->facets) {
                if(all_facet_vertices_are_marked(*mesh_grob(),f,v_is_picked)) {
                    f_to_new[f] = first_new_f + nb_new_f;
                    ++nb_new_f;
                }
            }
            if(mesh_grob()->facets.are_simplices()) {
                mesh_grob()->facets.create_triangles(nb_new_f);
            } else {
                for(index_t f=0; f<first_new_f; ++f) {
                    if(f_to_new[f] != NO_FACET) {
                        index_t nb_vertices =
                            mesh_grob()->facets.nb_vertices(f);
                        mesh_grob()->facets.create_polygon(nb_vertices);
                    }
                }
            }
            for(index_t f=0; f<first_new_f; ++f) {
                if(f_to_new[f] != NO_FACET) {
                    index_t nb_vertices = mesh_grob()->facets.nb_vertices(f);
                    index_t new_f = f_to_new[f];
                    for(index_t lv=0; lv<nb_vertices; ++lv) {
                        index_t v = mesh_grob()->facets.vertex(f, lv);
                        v = v_to_new[v];
                        mesh_grob()->facets.set_vertex(new_f, lv, v);
                        index_t adj = mesh_grob()->facets.adjacent(f,lv);
                        if(adj != NO_FACET) {
                            adj = f_to_new[adj];
                        }
                        mesh_grob()->facets.set_adjacent(new_f,lv,adj);
                    }
                }
            }
        }

        if(mesh_grob()->cells.nb() != 0) {
            index_t first_new_c = mesh_grob()->cells.nb();
            vector<index_t> c_to_new(mesh_grob()->cells.nb(), NO_CELL);
            index_t nb_new_c = 0;
            for(index_t c: mesh_grob()->cells) {
                if(all_cell_vertices_are_marked(*mesh_grob(),c,v_is_picked)) {
                    c_to_new[c] = first_new_c + nb_new_c;
                    ++nb_new_c;
                }
            }
            if(mesh_grob()->cells.are_simplices()) {
                mesh_grob()->cells.create_tets(nb_new_c);
            } else {
                for(index_t c=0; c<first_new_c; ++c) {
                    if(c_to_new[c] != NO_CELL) {
                        MeshCellType type = mesh_grob()->cells.type(c);
                        mesh_grob()->cells.create_cells(1,type);
                    }
                }
            }
            for(index_t c=0; c<first_new_c; ++c) {
                if(c_to_new[c] != NO_CELL) {
                    index_t new_c = c_to_new[c];
                    index_t nb_vertices = mesh_grob()->cells.nb_vertices(c);
                    for(index_t lv=0; lv<nb_vertices; ++lv) {
                        index_t v = mesh_grob()->cells.vertex(c, lv);
                        v = v_to_new[v];
                        mesh_grob()->cells.set_vertex(new_c, lv, v);
                    }
                    index_t nb_facets = mesh_grob()->cells.nb_facets(c);
                    for(index_t lf=0; lf<nb_facets; ++lf) {
                        index_t adj = mesh_grob()->cells.adjacent(c,lf);
                        if(adj != NO_CELL) {
                            adj = c_to_new[adj];
                        }
                        mesh_grob()->cells.set_adjacent(new_c,lf,adj);
                    }
                }
            }
        }

        mesh_grob()->update();
    }

    void MeshGrobCopyComponent::transform_subset(const mat4& M) {
        for(index_t v=first_new_vertex_; v<mesh_grob()->vertices.nb(); ++v) {
            vec3& p = mesh_grob()->vertices.point(v);
            p = transform_point(p,M);
        }
    }



/**********************************************************/

    void MeshGrobTransformComponent::pick_subset(
        MeshGrobTransformSubset* tool, const RayPick& rp
    ) {
        if(mesh_grob() == nullptr) {
            return;
        }

        pick_component(tool, rp, v_is_picked_);

        index_t count = 0;
        center_ = vec3(0.0, 0.0, 0.0);
        for(index_t v: mesh_grob()->vertices) {
            if(v_is_picked_[v]) {
                const vec3& p = mesh_grob()->vertices.point(v);
                center_ += p;
                ++count;
            }
        }
        if(count != 0) {
            center_ = (1.0 / double(count)) * center_;
        }
    }

    void MeshGrobTransformComponent::transform_subset(const mat4& M) {
        if(mesh_grob() == nullptr || v_is_picked_.size() == 0) {
            return;
        }
        for(index_t v: mesh_grob()->vertices) {
            if(v_is_picked_[v]) {
                vec3& p = mesh_grob()->vertices.point(v);
                p = transform_point(p,M);
            }
        }
    }

    void MeshGrobTransformComponent::clear_subset() {
        v_is_picked_.clear();
    }

    /*********************************************************/

    MeshGrobKeepOrRemoveComponent::~MeshGrobKeepOrRemoveComponent() {
    }

    /*********************************************************/

    void MeshGrobFlipComponent::grab(const RayPick& rp) {
        MeshGrobTool::grab(rp);
	index_t picked_facet = pick_facet(rp);
	if(picked_facet != NO_CELL) {
	    vector<bool> f_is_marked(mesh_grob()->facets.nb(), false);
	    std::stack<index_t> S;
	    f_is_marked[picked_facet] = true;
	    S.push(picked_facet);
	    while(!S.empty()) {
		index_t f = S.top();
		S.pop();
		for(index_t le=0; le<mesh_grob()->facets.nb_vertices(f); ++le) {
		    index_t adj = mesh_grob()->facets.adjacent(f,le);
		    if(adj != NO_FACET && !f_is_marked[adj]) {
			f_is_marked[adj] = true;
			S.push(adj);
		    }
		}
		mesh_grob()->facets.flip(f);
	    }
	    mesh_grob()->update();
	}
    }

}
