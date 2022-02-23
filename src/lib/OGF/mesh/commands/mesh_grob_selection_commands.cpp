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
 *  (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#include <OGF/mesh/commands/mesh_grob_selection_commands.h>
#include <geogram/points/colocate.h>

namespace OGF {

    MeshGrobSelectionCommands::MeshGrobSelectionCommands() {
    }

    MeshGrobSelectionCommands::~MeshGrobSelectionCommands() {
    }
    
    void MeshGrobSelectionCommands::select_all_vertices() {
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
        for(index_t v: mesh_grob()->vertices) {
            v_selection[v] = true;
        }
        mesh_grob()->update();
    }

    void MeshGrobSelectionCommands::unselect_all_vertices() {
        if(mesh_grob()->vertices.attributes().is_defined("selection")) {
            mesh_grob()->vertices.attributes().delete_attribute_store(
                "selection"
            );
        }
        mesh_grob()->update();        
    }

    void MeshGrobSelectionCommands::invert_vertices_selection() {
        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );
        for(index_t v: mesh_grob()->vertices) {
            v_selection[v] = !v_selection[v];
        }
        mesh_grob()->update();        
    }
    
    void MeshGrobSelectionCommands::select_vertices_on_surface_border() {
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
        mesh_grob()->update();        
    }

    void MeshGrobSelectionCommands::unselect_vertices_on_surface_border() {
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
        mesh_grob()->update();        
    }

    void MeshGrobSelectionCommands::delete_selected_vertices(bool remove_isolated) {
        if(!mesh_grob()->vertices.attributes().is_defined("selection")) {
            return;
        }

        Attribute<bool> v_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );

        {
            vector<index_t> delete_e(mesh_grob()->edges.nb(),0);
            for(index_t e: mesh_grob()->edges) {
                if(
                    v_selection[mesh_grob()->edges.vertex(e,0)] ||
                    v_selection[mesh_grob()->edges.vertex(e,0)]
                ) {
                    delete_e[e] = 1;
                }
            }
            mesh_grob()->edges.delete_elements(delete_e, remove_isolated);
        }

        {
            vector<index_t> delete_f(mesh_grob()->facets.nb(),0);
            for(index_t f: mesh_grob()->facets) {
                for(index_t lv=0; lv<mesh_grob()->facets.nb_vertices(f); ++lv) {
                    if(v_selection[mesh_grob()->facets.vertex(f,lv)]) {
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
                for(index_t lv=0; lv<mesh_grob()->cells.nb_vertices(c); ++lv) {
                    if(v_selection[mesh_grob()->cells.vertex(c,lv)]) {
                        delete_c[c] = 1;
                        break;
                    }
                }
            }
            mesh_grob()->cells.delete_elements(delete_c, remove_isolated);
        }

        {
            vector<index_t> delete_v(mesh_grob()->vertices.nb(),0);
            for(index_t v: mesh_grob()->vertices) {
                if(v_selection[v]) {
                    delete_v[v] = 1;
                }
            }
            mesh_grob()->vertices.delete_elements(delete_v);
        }
        
        mesh_grob()->update();
    }


    void MeshGrobSelectionCommands::select_duplicated_vertices(double tolerance) {
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
	mesh_grob()->update();
    }
}

