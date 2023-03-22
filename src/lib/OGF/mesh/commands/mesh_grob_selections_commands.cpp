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
 * As an exception to the GPL, Graphite can be linked with the following 
 *  (non-GPL) libraries:  Qt, SuperLU, WildMagic and CGAL
 */

#include <OGF/mesh/commands/mesh_grob_selections_commands.h>
#include <OGF/mesh/commands/filter.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <geogram/mesh/mesh_surface_intersection.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/numerics/predicates.h>
#include <geogram/points/colocate.h>

namespace OGF {
    
    MeshGrobSelectionsCommands::MeshGrobSelectionsCommands() {
    }

    MeshGrobSelectionsCommands::~MeshGrobSelectionsCommands() {
    }

    void MeshGrobSelectionsCommands::hide_selection() {
        hide_attribute();
    }

    void MeshGrobSelectionsCommands::select_all() {
        MeshElementsFlags where = visible_selection();
        if(where == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );
        for(index_t i=0; i<selection.size(); ++i) {
            selection[i] = true;
        }
        mesh_grob()->update();
    }

    void MeshGrobSelectionsCommands::select_none() {
        MeshElementsFlags where = visible_selection();
        if(where == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );
        for(index_t i=0; i<selection.size(); ++i) {
            selection[i] = false;
        }
        mesh_grob()->update();
    }

    void MeshGrobSelectionsCommands::invert_selection() {
        MeshElementsFlags where = visible_selection();
        if(visible_selection() == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );
        for(index_t i=0; i<selection.size(); ++i) {
            selection[i] = !selection[i];
        }
        mesh_grob()->update();
    }

    void MeshGrobSelectionsCommands::enlarge_selection(index_t nb_times) {
        MeshElementsFlags where = visible_selection();
        if(where == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }

        for(index_t k=0; k<nb_times; ++k) {
            Attribute<bool> selection(
                mesh_grob()->get_subelements_by_type(where).attributes(),
                "selection"
            );
        
            vector<bool> new_selection(
                mesh_grob()->get_subelements_by_type(where).nb(), false
            );
            
            switch(where) {
            case MESH_VERTICES: {
                for(index_t f: mesh_grob()->facets) {
                    index_t N = mesh_grob()->facets.nb_vertices(f);
                    for(index_t lv1=0; lv1<N; ++lv1){
                        index_t lv2=(lv1+1) % N;
                        index_t v1 = mesh_grob()->facets.vertex(f,lv1);
                        index_t v2 = mesh_grob()->facets.vertex(f,lv2);
                        if(selection[v1] || selection[v2]) {
                            new_selection[v1] = true;
                            new_selection[v2] = true;
                        }
                    }
                }
                for(index_t c: mesh_grob()->cells) {
                    for(
                        index_t lf = 0; lf<mesh_grob()->cells.nb_facets(c); ++lf
                    ) {
                        index_t N = mesh_grob()->cells.facet_nb_vertices(c,lf);
                        for(index_t lv1=0; lv1<N; ++lv1) {
                            index_t lv2=(lv1+1) % N;
                            index_t v1 =
                                mesh_grob()->cells.facet_vertex(c,lf,lv1);
                            index_t v2 =
                                mesh_grob()->cells.facet_vertex(c,lf,lv2);
                            if(selection[v1] || selection[v2]) {
                                new_selection[v1] = true;
                                new_selection[v2] = true;
                            }
                        }
                    }
                }
            } break;
            case MESH_FACETS: {
                for(index_t f: mesh_grob()->facets) {
                    if(selection[f]) {
                        new_selection[f] = true;
                    }
                    for(
                        index_t le=0;
                        le<mesh_grob()->facets.nb_vertices(f); ++le
                    ) {
                        index_t g = mesh_grob()->facets.adjacent(f,le);
                        if(g != index_t(-1) && selection[g]) {
                            new_selection[f] = true;
                        }
                    }
                }
            } break;
            case MESH_CELLS: {
                for(index_t c: mesh_grob()->cells) {
                    if(selection[c]) {
                        new_selection[c] = true;
                    }
                    for(
                        index_t lf=0; lf<mesh_grob()->cells.nb_facets(c); ++lf
                    ) {
                        index_t d = mesh_grob()->cells.adjacent(c,lf);
                        if(d != index_t(-1) && selection[d]) {
                            new_selection[c] = true;
                        }
                    }
                }
            } break;
            default: {
                Logger::err("Selection") << "Invalid localisation"
                                         << std::endl;
            } break;
            }
            
            for(index_t i: mesh_grob()->get_subelements_by_type(where)) {
                selection[i] = new_selection[i];
            }
        }
        
        mesh_grob()->update();
    }
    
    void MeshGrobSelectionsCommands::shrink_selection(index_t nb_times) {
        MeshElementsFlags where = visible_selection();
        if(where == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        // Feeling lazy today !
        // (shrink selection <=> enlarge complement of selection)
        invert_selection();
        enlarge_selection(nb_times);
        invert_selection();
        mesh_grob()->update();
    }
    

    void MeshGrobSelectionsCommands::close_small_holes_in_selection(
        index_t hole_size
    ) {
        MeshElementsFlags where = visible_selection();
        if(where == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        for(index_t i=0; i<hole_size; ++i) {
            enlarge_selection();
        }
        for(index_t i=0; i<hole_size; ++i) {
            shrink_selection();
        }
    }
    
    void MeshGrobSelectionsCommands::delete_selected_elements(
        bool remove_isolated
    ) {
        MeshElementsFlags where = visible_selection();
        if(visible_selection() == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );

        // Particular case: vertices. Delete all edges, facets, cells 
        // incident to a vertex to delete.
        if(where == MESH_VERTICES) {
            {
                vector<index_t> delete_e(mesh_grob()->edges.nb(),0);
                for(index_t e: mesh_grob()->edges) {
                    if(
                        selection[mesh_grob()->edges.vertex(e,0)] ||
                        selection[mesh_grob()->edges.vertex(e,0)]
                    ) {
                        delete_e[e] = 1;
                    }
                }
                mesh_grob()->edges.delete_elements(delete_e, remove_isolated);
            }

            {
                vector<index_t> delete_f(mesh_grob()->facets.nb(),0);
                for(index_t f: mesh_grob()->facets) {
                    for(index_t lv=0;
                        lv<mesh_grob()->facets.nb_vertices(f); ++lv
                    ) {
                        if(selection[mesh_grob()->facets.vertex(f,lv)]) {
                            delete_f[f] = 1;
                            break;
                        }
                    }
                }
                mesh_grob()->facets.delete_elements(delete_f, remove_isolated);
            }

            {
                vector<index_t> delete_c(mesh_grob()->cells.nb(),0);
                for(index_t c: mesh_grob()->cells) {
                    for(index_t lv=0;
                        lv<mesh_grob()->cells.nb_vertices(c); ++lv
                    ) {
                        if(selection[mesh_grob()->cells.vertex(c,lv)]) {
                            delete_c[c] = 1;
                            break;
                        }
                    }
                }
                mesh_grob()->cells.delete_elements(delete_c, remove_isolated);
            }
        }

        vector<index_t> remove_element(selection.size(), 0);
        for(index_t i=0; i<selection.size(); ++i) {
            remove_element[i] = index_t(selection[i]);
        }

        MeshElements& elts = dynamic_cast<MeshElements&>(
            mesh_grob()->get_subelements_by_type(where)
        );
        elts.delete_elements(remove_element, remove_isolated);
        mesh_grob()->update();
    }

    
    void MeshGrobSelectionsCommands::show_vertices_selection() {
        Attribute<bool> sel(mesh_grob()->vertices.attributes(),"selection");
        show_attribute("vertices.selection");
    }

    void MeshGrobSelectionsCommands::select_vertices_on_surface_border() {
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
        for(index_t f: mesh_grob()->facets) {
            for(index_t c: mesh_grob()->facets.corners(f)) {
                if(mesh_grob()->facet_corners.adjacent_facet(c) == NO_FACET) {
                    v_selection[mesh_grob()->facet_corners.vertex(c)] = true;
                }
            }
        }
        show_vertices_selection();
        mesh_grob()->update();        
    }

    void MeshGrobSelectionsCommands::unselect_vertices_on_surface_border() {
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
        for(index_t f: mesh_grob()->facets) {
            for(index_t c: mesh_grob()->facets.corners(f)) {
                if(mesh_grob()->facet_corners.adjacent_facet(c) == NO_FACET) {
                    v_selection[mesh_grob()->facet_corners.vertex(c)] = false;
                }
            }
        }
        show_vertices_selection();        
        mesh_grob()->update();        
    }

    void MeshGrobSelectionsCommands::select_duplicated_vertices(
	double tolerance
    ) {
	vector<index_t> old2new(mesh_grob()->vertices.nb());
	index_t nb_distinct;
	if(tolerance == 0.0) {
	    nb_distinct = Geom::colocate_by_lexico_sort(
		mesh_grob()->vertices.point_ptr(0),
		coord_index_t(mesh_grob()->vertices.dimension()),
		mesh_grob()->vertices.nb(),
		old2new,
		mesh_grob()->vertices.dimension()
	    );
	} else {
	    nb_distinct = Geom::colocate(
		mesh_grob()->vertices.point_ptr(0),
		coord_index_t(mesh_grob()->vertices.dimension()),
		mesh_grob()->vertices.nb(),
		old2new,
		tolerance
	    );
	}

	Logger::out("Colocate") << mesh_grob()->vertices.nb() - nb_distinct
				<< " colocated vertices"
				<< std::endl;
	
	vector<index_t> new_count(mesh_grob()->vertices.nb(),0);
	for(index_t v: mesh_grob()->vertices) {
	    ++new_count[old2new[v]];
	}
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
	for(index_t v: mesh_grob()->vertices) {
	    if(new_count[old2new[v]] > 1) {
		v_selection[v] = true;
	    }
	}
        show_vertices_selection();        
	mesh_grob()->update();
    }

    void MeshGrobSelectionsCommands::select_vertices_on_degenerate_facets() {
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
        for(index_t f: mesh_grob()->facets) {
            if(mesh_grob()->facets.nb_vertices(f) == 3) {
                index_t v1 = mesh_grob()->facets.vertex(f,0);
                index_t v2 = mesh_grob()->facets.vertex(f,1);
                index_t v3 = mesh_grob()->facets.vertex(f,2);
                vec3 p1(mesh_grob()->vertices.point_ptr(v1));
                vec3 p2(mesh_grob()->vertices.point_ptr(v2));
                vec3 p3(mesh_grob()->vertices.point_ptr(v3));
                if(PCK::aligned_3d(p1,p2,p3)) {
                    v_selection[v1] = true;
                    v_selection[v2] = true;
                    v_selection[v3] = true;                    
                }
            }
        }
        show_vertices_selection();        
	mesh_grob()->update();
    }
    
    void MeshGrobSelectionsCommands::show_facets_selection() {
        Attribute<bool> sel(mesh_grob()->facets.attributes(),"selection");
        show_attribute("facets.selection");
        hide_vertices();
    }

    void MeshGrobSelectionsCommands::select_degenerate_facets(
        bool add_to_selection
    ) {
        Attribute<bool> sel(mesh_grob()->facets.attributes(),"selection");
        if(!add_to_selection) {
            for(index_t f:mesh_grob()->facets) {
                sel[f] = false;
            }
        }

        bool has_degenerate_facets = false;
        
        for(index_t f: mesh_grob()->facets) {
            bool degenerate = true;
            index_t N = mesh_grob()->facets.nb_vertices(f);
            for(index_t li=0; li<N; ++li) {
                index_t i = mesh_grob()->facets.vertex(f,li);
                vec3 p1(mesh_grob()->vertices.point_ptr(i));
                for(index_t lj=li+1; lj<N; ++lj) {
                    index_t j = mesh_grob()->facets.vertex(f,lj);
                    vec3 p2(mesh_grob()->vertices.point_ptr(j));
                    for(index_t lk=lj+1; lk<N; ++lk) {
                        index_t k = mesh_grob()->facets.vertex(f,lk);
                        vec3 p3(mesh_grob()->vertices.point_ptr(k));
                        degenerate = degenerate && PCK::aligned_3d(p1,p2,p3);
                        has_degenerate_facets = has_degenerate_facets || degenerate;
                    }
                }
            }
            sel[f] = sel[f] || degenerate;
        }
        show_facets_selection();
        if(has_degenerate_facets) {
            show_vertices();
        }
        mesh_grob()->update();
    }

    
    void MeshGrobSelectionsCommands::select_intersecting_facets(
        bool add_to_selection, bool test_adjacent_facets
    ) {
        Attribute<bool> sel(mesh_grob()->facets.attributes(),"selection");
        if(!add_to_selection) {
            for(index_t f:mesh_grob()->facets) {
                sel[f] = false;
            }
        }

        bool has_intersections = false;
        
        MeshFacetsAABB AABB(*mesh_grob());
        vector<std::pair<index_t, index_t> > candidates;
        AABB.compute_facet_bbox_intersections(
            [&](index_t f1, index_t f2) {
                if(f1 == f2) {
                    return;
                }
                if(
                    !test_adjacent_facets && (
                        (mesh_grob()->
                            facets.find_adjacent(f1,f2)       != index_t(-1)) ||
                        (mesh_grob()->
                             facets.find_common_vertex(f1,f2) != index_t(-1))
                    )
                ) {
                    return;
                }
                candidates.push_back(std::make_pair(f1,f2));
            }
        );

        parallel_for(
            0, candidates.size(),
            [&](index_t i) {
                index_t f1 = candidates[i].first;
                index_t f2 = candidates[i].second;                
                if(mesh_facets_have_intersection(*mesh_grob(), f1, f2)) {
                    sel[f1] = true;
                    sel[f2] = true;
                    has_intersections = true;
                }
            }
        );

        show_facets_selection();
        if(has_intersections) {
            show_vertices();
        }
        mesh_grob()->update();
    }

    void MeshGrobSelectionsCommands::select_facets_from_vertices_selection() {
        Attribute<bool> sel(mesh_grob()->facets.attributes(),"selection");
        Attribute<bool> vsel(mesh_grob()->vertices.attributes(),"selection");
        for(index_t f: mesh_grob()->facets) {
            bool f_is_selected = true;
            for(index_t lv=0; lv<mesh_grob()->facets.nb_vertices(f); ++lv) {
                f_is_selected =
                    f_is_selected && vsel[mesh_grob()->facets.vertex(f,lv)];
                if(!f_is_selected) {
                    break;
                }
            }
            sel[f] = f_is_selected;
        }
        show_attribute("facets.selection");
        hide_vertices();
        mesh_grob()->update();
    }

    void MeshGrobSelectionsCommands::select_duplicated_facets() {
        Mesh M;
        M.copy(*mesh_grob());
        Attribute<index_t> orig_facet(M.facets.attributes(), "orig_facet");
        for(index_t f: M.facets) {
            orig_facet[f] = f;
        }
        mesh_repair(M, MESH_REPAIR_DUP_F);
        Attribute<bool> selection(mesh_grob()->facets.attributes(), "selection");
        for(index_t f: mesh_grob()->facets) {
            selection[f] = true;
        }
        for(index_t f: M.facets) {
            selection[orig_facet[f]] = false;
        }
        show_facets_selection();        
	mesh_grob()->update();
    }
    
    void MeshGrobSelectionsCommands::show_cells_selection() {
        Attribute<bool> sel(mesh_grob()->cells.attributes(),"selection");
        show_attribute("cells.selection");
        hide_vertices();        
    }

    MeshElementsFlags MeshGrobSelectionsCommands::visible_selection() const {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob()->get_shader()
        );
        
        if(shd == nullptr) {
            return MESH_NONE;
        }

        std::string painting;
        shd->get_property("painting",painting);
        if(painting != "ATTRIBUTE") {
            return MESH_NONE;
        }
        
        std::string full_attribute_name;
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        shd->get_property("attribute",full_attribute_name);
        if(!Mesh::parse_attribute_name(
               full_attribute_name,where,attribute_name,component)
          ) {
            return MESH_NONE;
        }
        
        return where;
    }

    void MeshGrobSelectionsCommands::set_selection(
        const std::string& selection_string
    ) {
        MeshElementsFlags where = visible_selection();
        if(visible_selection() == MESH_NONE) {
            Logger::err("Selection") << "No visible selection"
                                     << std::endl;
            return;
        }
        Attribute<bool> selection(
            mesh_grob()->get_subelements_by_type(where).attributes(),
            "selection"
        );
        try {
            Filter filter(selection.size(), selection_string);
            for(index_t i=0; i<selection.size(); ++i) {
                selection[i] = filter.test(i);
            }
        } catch(...) {
            Logger::err("Attributes") << "Invalid filter specification"
                                      << std::endl;
        }
        mesh_grob()->update();
    }

}

