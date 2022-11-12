
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

#include <OGF/mesh/tools/mesh_grob_border_tools.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <geogram/mesh/mesh_local_operations.h>
#include <geogram/mesh/mesh_halfedges.h>

namespace {
    using namespace OGF;

    /**
     * \brief Tests whether two facet edges have the same geometric
     *  vertices.
     * \details The test is geometric (the vertices may have different
     *  indices).
     * \param[in] M a const reference to the Mesh.
     * \param[in] f1 the index of the first facet
     * \param[in] c1 the index of the first corner
     * \param[in] f2 the index of the second facet
     * \param[in] c2 the index of the second corner
     * \retval true if the edge that starts from (f1,c1) matches
     *   the one that starts from (f2,c2)
     * \retval false otherwise
     */
    bool mesh_edges_match(
        const Mesh& M,
        index_t f1, index_t c1,
        index_t f2, index_t c2
    ) {
        index_t c1n = M.facets.next_corner_around_facet(f1,c1);
        index_t v1 = M.facet_corners.vertex(c1);
        index_t v2 = M.facet_corners.vertex(c1n);
        const double* p1 = M.vertices.point_ptr(v1);
        const double* p2 = M.vertices.point_ptr(v2);
        
        index_t c2n = M.facets.next_corner_around_facet(f2,c2);
        index_t w1 = M.facet_corners.vertex(c2);
        index_t w2 = M.facet_corners.vertex(c2n);
        const double* q1 = M.vertices.point_ptr(w1);
        const double* q2 = M.vertices.point_ptr(w2);

        for(index_t c=0; c<M.vertices.dimension(); ++c) {
            if(q1[c] != p2[c] || q2[c] != p1[c]) {
                return false;
            }
        }

        return true;
    }
}

namespace OGF {

    void MeshGrobGlueEdges::grab(const RayPick& p_ndc) {
        index_t f[2];
        index_t c[2];
        if(!pick_facet_edge(p_ndc, f[0], c[0])) {
            return;
        }

        if(mesh_grob()->facet_corners.adjacent_facet(c[0]) != NO_FACET) {
            Logger::err("Tool") << "Picked edge is not on border"
                                << std::endl;
            return;
        }

        f[1] = NO_FACET;
        c[1] = NO_CORNER;
        
        for(index_t ff: mesh_grob()->facets) {
            if(ff == f[0]) {
                continue;
            }
            for(index_t cc: mesh_grob()->facets.corners(ff)) {
                if(mesh_grob()->facet_corners.adjacent_facet(cc) != NO_FACET) {
                    continue;
                }
                if(mesh_edges_match(*mesh_grob(),f[0],c[0],ff,cc)) {
                    if(f[1] != NO_FACET) {
                        Logger::err("Tool")
                            << "More than one edge match the picked edge."
                            << std::endl;
                        return;
                    }
                    f[1] = ff;
                    c[1] = cc;
                }
            }
        }

        if(f[1] == NO_FACET) {
            Logger::err("Tool")
                << "Did not find another edge that matches the picked one"
                << std::endl;
            return;
        }
        
        glue_edges(*mesh_grob(),f[0], c[0], f[1], c[1]);
        
        mesh_grob()->update();
    }

    void MeshGrobUnglueEdges::grab(const RayPick& p_ndc) {

        // Step 1: pick edge (f1,c1) and determine opposite edge (f2,c2)
        
        index_t f1;
        index_t c1;
        if(!pick_facet_edge(p_ndc, f1, c1)) {
            return;
        }
        index_t f2 = mesh_grob()->facet_corners.adjacent_facet(c1);
        if(f2 == NO_FACET) {
            Logger::err("Tool") << "Picked edge is already on border"
                                << std::endl;
            return;
        }

        unglue_edges(*mesh_grob(), f1, c1);
        
        mesh_grob()->update();
    }


    void MeshGrobZipEdges::grab(const RayPick& p_ndc) {
        index_t v = pick_vertex(p_ndc);
        if(v == NO_VERTEX) {
            return;
        }
        index_t f1 = NO_FACET;
        index_t c1 = NO_CORNER;
        for(index_t f: mesh_grob()->facets) {
            for(index_t c: mesh_grob()->facets.corners(f)) {
                index_t adj = mesh_grob()->facet_corners.adjacent_facet(c);
                if(adj != NO_FACET) {
                    continue;
                }
                if(mesh_grob()->facet_corners.vertex(c) == v) {
                    if(f1 == NO_FACET) {
                        f1 = f;
                        c1 = c;
                    } else {
                        Logger::err("Tool")
                            << "More than two borders incident to this vertex"
                            << std::endl;
                        return;
                    }
                }
            }
        }
        MeshHalfedges MH(*mesh_grob());
        MeshHalfedges::Halfedge H(f1,c1);
        MeshHalfedges::Halfedge Hprev(H);
        MH.move_to_prev_around_border(Hprev);
        MeshHalfedges::Halfedge Htest(Hprev);
        MH.move_to_prev_around_border(Htest);
        MH.move_to_prev_around_border(Htest);
        if(Htest == H) {
            Logger::err("Tool")
                << "Cannot zip last edge of triangle hole"
                << std::endl;
            return;
        }
        
        glue_edges(*mesh_grob(), f1, c1, Hprev.facet, Hprev.corner);
        mesh_grob()->update();
    }

    void MeshGrobGlueUnglueEdges::reset() {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            object()->get_shader()
        );
        if(shd != nullptr) {
            shd->show_mesh();
            shd->show_borders();
        }        
        MultiTool::reset();
    }

    
    void MeshGrobZipUnzipEdges::reset() {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            object()->get_shader()
        );
        if(shd != nullptr) {
            shd->show_vertices();            
            shd->show_mesh();
            shd->show_borders();
        }        
        MultiTool::reset();
    }

    
    void MeshGrobConnectEdges::grab(const RayPick& p_ndc) {
        if(f_ == NO_FACET || c_ == NO_CORNER) {
            pick_facet_edge(p_ndc, f_, c_);
            
            if(mesh_grob()->facet_corners.adjacent_facet(c_) != NO_FACET) {
                Logger::err("Tool")
                    << "Picked edge is not on the border"
                    << std::endl;
                reset();
            }
            
            return;
        }

        index_t f2 = NO_FACET;
        index_t c2 = NO_FACET;
        if(pick_facet_edge(p_ndc, f2, c2)) {
            if(mesh_grob()->facet_corners.adjacent_facet(c2) != NO_FACET) {
                Logger::err("Tool")
                    << "Picked edge is not on the border"
                    << std::endl;
                reset();
                return;
            }
        }

        if(f_ == f2) {
            Logger::err("Tool")
                << "Picked edges are on, the same facet"
                << std::endl;
            reset();
            return;
        }

        glue_edges(*mesh_grob(), f_, c_, f2, c2);
        mesh_grob()->update();
        reset();
    }

    void MeshGrobConnectEdges::reset() {
        f_ = NO_FACET;
        c_ = NO_CORNER;
    }
    
    void MeshGrobConnectDisconnectEdges::reset() {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            object()->get_shader()
        );
        if(shd != nullptr) {
            shd->show_mesh();
            shd->show_borders();
        }        
        MultiTool::reset();
    }
}

