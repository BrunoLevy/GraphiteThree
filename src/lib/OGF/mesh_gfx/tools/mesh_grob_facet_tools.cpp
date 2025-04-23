
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

#include <OGF/mesh_gfx/tools/mesh_grob_facet_tools.h>
#include <OGF/mesh_gfx/shaders/mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/mesh/mesh_geometry.h>

namespace {
    using namespace OGF;

    /**
     * \brief Connects two facets in a MeshGrob.
     * \details This updates two corner adjacent facet links, given
     *   facet indices and corner indices.
     * \param[in] mesh_grob a pointer to the MeshGrob.
     * \param[in] f1 the index of the first facet
     * \param[in] c1 the index of the first corner, in the first facet
     * \param[in] f2 the index of the second facet
     * \param[in] c2 the index of the second corner, in the second facet
     */
    void connect_facets(
        MeshGrob* mesh_grob,
        index_t f1, index_t c1, index_t f2, index_t c2
    ) {
        geo_debug_assert(f1 < mesh_grob->facets.nb());
        geo_debug_assert(f2 < mesh_grob->facets.nb());
        geo_debug_assert(c1 >= mesh_grob->facets.corners_begin(f1));
        geo_debug_assert(c1 < mesh_grob->facets.corners_end(f1));
        geo_debug_assert(c2 >= mesh_grob->facets.corners_begin(f2));
        geo_debug_assert(c2 < mesh_grob->facets.corners_end(f2));
        mesh_grob->facet_corners.set_adjacent_facet(c1,f2);
        mesh_grob->facet_corners.set_adjacent_facet(c2,f1);
    }

    /**
     * \brief Connects two facets in a MeshGrob.
     * \details This updates two corner adjacent facet links. The corner
     *  of the second facet is retreived by comparing vertices ids.
     * \param[in] mesh_grob a pointer to the MeshGrob.
     * \param[in] f1 the index of the first facet
     * \param[in] c1 the index of the first corner, in the first facet
     * \param[in] f2 the index of the second facet
     */
    void connect_facets(
        MeshGrob* mesh_grob,
        index_t f1, index_t c1, index_t f2
    ) {
        index_t c1n = mesh_grob->facets.next_corner_around_facet(f1,c1);
        index_t v = mesh_grob->facet_corners.vertex(c1n);

        if(f2 == NO_FACET) {
            mesh_grob->facet_corners.set_adjacent_facet(c1,NO_FACET);
            return;
        }

        for(
            index_t c2 = mesh_grob->facets.corners_begin(f2);
            c2 < mesh_grob->facets.corners_end(f2);
            ++c2
        ) {
            if(mesh_grob->facet_corners.vertex(c2) == v) {
                connect_facets(mesh_grob,f1,c1,f2,c2);
                return;
            }
        }

        geo_assert_not_reached;
    }


    /**
     * \brief A closed loop of halfedges.
     */
    typedef vector<MeshHalfedges::Halfedge> Loop;

    /**
     * \brief Creates a facet that will replace a Loop in a
     *  MeshGrob.
     * \details The created facet will have the same orientation as
     *  the loop. The loop is composed of facet corners, that will be
     *  destroyed subsequently (else this would clearly create non-manifold
     *  configurations).
     * \param[in] mesh_grob a pointer to the MeshGrob
     * \param[in] loop a const reference to the loop
     */
    index_t create_facet_replace_loop(
        MeshGrob* mesh_grob, const Loop& loop
    ) {
        index_t f = mesh_grob->facets.create_polygon(loop.size());

        // Set vertices
        for(index_t lv=0; lv<loop.size(); ++lv) {
            index_t v = mesh_grob->facet_corners.vertex(loop[lv].corner);
            mesh_grob->facets.set_vertex(f, lv, v);
        }

        // Set adjacent facet
        for(index_t lv=0; lv<loop.size(); ++lv) {
            index_t adj = mesh_grob->facet_corners.adjacent_facet(
                loop[lv].corner
            );
            connect_facets(
                mesh_grob, f, mesh_grob->facets.corners_begin(f) + lv, adj
            );
        }

        return f;
    }


    /**
     * \brief Tests whether a facet of a mesh is incident to the
     *  same vertex more than once.
     * \param[in] mesh_grob a const pointer to the MeshGrob
     * \param[in] f the index of the facet
     * \retval true if facet \p f in \p mesh_grob is incident to the
     *  same vertex more than once
     * \retval false otherwise
     */
    bool facet_has_bowtie(
        const MeshGrob* mesh_grob,
        index_t f
    ) {
        std::set<index_t> vertices;
        for(
            index_t lv=0; lv< mesh_grob->facets.nb_vertices(f);
            ++lv
        ) {
            index_t v = mesh_grob->facets.vertex(f, lv);
            if(vertices.find(v) != vertices.end()) {
                return true;
            }
            vertices.insert(v);
        }
        return false;
    }
}

namespace OGF {

    void MeshGrobCreateCenterVertex::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        index_t picked_facet = pick_facet(p_ndc);
        if(picked_facet != NO_FACET) {
            new_vertex_ = mesh_grob()->vertices.create_vertex(
                picked_point().data()
            );

            index_t first_facet = NO_FACET;
            index_t prev_facet = NO_FACET;
            for(
                index_t c1=mesh_grob()->facets.corners_begin(picked_facet);
                c1 < mesh_grob()->facets.corners_end(picked_facet);
                ++c1
            ) {
                index_t c2 = mesh_grob()->facets.next_corner_around_facet(
                    picked_facet, c1
                );

                index_t v1 = mesh_grob()->facet_corners.vertex(c1);
                index_t v2 = mesh_grob()->facet_corners.vertex(c2);
                index_t nf = mesh_grob()->facets.create_triangle(
                    v1,v2,new_vertex_
                );

                connect_facets(
                    mesh_grob(),
                    nf, mesh_grob()->facets.corners_begin(nf),
                    mesh_grob()->facet_corners.adjacent_facet(c1)
                );

                if(first_facet == NO_FACET) {
                    first_facet = nf;
                } else {
                    connect_facets(
                        mesh_grob(),
                        prev_facet,
                        mesh_grob()->facets.corners_begin(prev_facet) + 1,
                        nf
                    );
                }
                prev_facet = nf;
            }

            connect_facets(
                mesh_grob(),
                prev_facet,
                mesh_grob()->facets.corners_begin(prev_facet) + 1,
                first_facet
            );


            vector<index_t> to_delete(mesh_grob()->facets.nb());
            to_delete[picked_facet] = 1;
            mesh_grob()->facets.delete_elements(to_delete);

            mesh_grob()->update();
        } else {
            new_vertex_ = pick_vertex(p_ndc);
        }
    }

    void MeshGrobCreateCenterVertex::drag(const RayPick& p_ndc) {
        if(new_vertex_ != NO_VERTEX && mesh_grob()->vertices.dimension() >= 3) {
            mesh_grob()->vertices.point(new_vertex_) = drag_point(p_ndc);
            mesh_grob()->update();
        }
    }


    void MeshGrobRemoveCenterVertex::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        index_t picked_v = pick_vertex(p_ndc);
        if(picked_v == NO_VERTEX) {
            return;
        }
        index_t picked_f = NO_FACET;
        index_t picked_c = NO_CORNER;
        for(index_t f=0; f<mesh_grob()->facets.nb(); ++f) {
            for(
                index_t c=mesh_grob()->facets.corners_begin(f);
                c < mesh_grob()->facets.corners_end(f); ++c
            ) {
                if(mesh_grob()->facet_corners.vertex(c) == picked_v) {
                    picked_f = f;
                    picked_c = c;
                    break;
                }
            }
        }
        if(picked_c == NO_CORNER) {
            Logger::err("Tool")
                << "There is no facet incident to this vertex"
                << std::endl;
            return;
        }
        if(mesh_grob()->facet_corners.adjacent_facet(picked_c) == NO_FACET) {
            Logger::err("Tool")
                << "Cannot remove vertex on the border"
                << std::endl;
            return;
        }

        Loop loop;
        // +1 because we will create a new facet.
        vector<index_t> remove_f(mesh_grob()->facets.nb()+1,0);
        MeshHalfedges MH(*mesh_grob());
        MeshHalfedges::Halfedge firstH(picked_f, picked_c);

        // Tag the facets to be removed.
        MeshHalfedges::Halfedge H = firstH;
        do {
            remove_f[H.facet] = 1;
            MH.move_to_prev_around_facet(H);
            MH.move_to_opposite(H);
        } while(H != firstH);

        // Get the border of the facet to be created.
        H = firstH;
        do {
            MeshHalfedges::Halfedge HH = H;
            do {
                index_t adj =
                    mesh_grob()->facet_corners.adjacent_facet(HH.corner);
                if(adj == NO_FACET || !remove_f[adj]) {
                    loop.push_back(HH);
                }
                MH.move_to_next_around_facet(HH);
            } while(HH != H);

            MH.move_to_prev_around_facet(H);
            MH.move_to_opposite(H);
        } while(H != firstH);


        create_facet_replace_loop(mesh_grob(),loop);

        mesh_grob()->facets.delete_elements(remove_f);
        mesh_grob()->update();
    }


    void MeshGrobEditCenterVertex::reset() {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            object()->get_shader()
        );
        if(shd != nullptr) {
            shd->show_mesh();
            shd->show_vertices();
        }
        MultiTool::reset();
    }


    /*************************************************************/

    void MeshGrobRemoveIncidentFacets::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        index_t picked_vertex = pick_vertex(p_ndc);
        if(picked_vertex != NO_VERTEX) {
            vector<index_t> to_delete(mesh_grob()->facets.nb(),0);
            for(index_t f=0; f<mesh_grob()->facets.nb(); ++f) {
                for(index_t c = mesh_grob()->facets.corners_begin(f);
                    c < mesh_grob()->facets.corners_end(f);
                    ++c) {
                    if(mesh_grob()->facet_corners.vertex(c) == picked_vertex) {
                        to_delete[f] = 1;
                    }
                }
            }
            mesh_grob()->facets.delete_elements(to_delete);
            mesh_grob()->update();
        }
    }

    void MeshGrobRemoveIncidentFacets::reset() {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            object()->get_shader()
        );
        if(shd != nullptr) {
            shd->show_mesh();
            shd->show_vertices();
        }
        MeshGrobTool::reset();
    }

    /****************************************************************/

    void MeshGrobRemoveFacet::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        index_t result = pick_facet(p_ndc);
        if(result != index_t(-1)) {
            if(result > mesh_grob()->facets.nb()) {
                Logger::err("Tool") << "Wrong facet ID!!!" << std::endl;
                return;
            }
            vector<index_t> to_delete(mesh_grob()->facets.nb(),0);
            to_delete[result] = 1;
            mesh_grob()->facets.delete_elements(to_delete);
            mesh_grob()->update();
        }
    }

    /****************************************************************/

    void MeshGrobFillHole::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        index_t facet,corner;
        if(pick_facet_edge(p_ndc,facet,corner)) {
            if(mesh_grob()->facet_corners.adjacent_facet(corner) != NO_FACET) {
                Logger::err("Tool") << "Picked edge is not on border"
                                    << std::endl;
                return;
            }

            //   To detect pinchouts, count the number of
            // times each vertex appears in the hole.

            std::map<index_t, index_t> v_to_nb;
            MeshHalfedges MH(*mesh_grob());
            vector<MeshHalfedges::Halfedge> hole;
            MeshHalfedges::Halfedge first(facet, corner);
            MeshHalfedges::Halfedge H(facet, corner);
            do {
                index_t v = mesh_grob()->facet_corners.vertex(H.corner);
                ++v_to_nb[v];
                hole.push_back(H);
                MH.move_to_next_around_border(H);
            } while(H != first);

            //  Count the number of pinchouts.
            index_t nb_pinchouts = 0;
            for(index_t i=0; i<hole.size(); ++i) {
                index_t v = mesh_grob()->facet_corners.vertex(hole[i].corner);
                if(v_to_nb[v] > 1) {
                    ++nb_pinchouts;
                }
            }

            if(nb_pinchouts > 0) {
                vector<MeshHalfedges::Halfedge> clean_hole;
                bool in_hole_part = true;
                for(index_t i=0; i<hole.size(); ++i) {
                    index_t v = mesh_grob()->facet_corners.vertex(
                        hole[i].corner
                    );
                    if(i != 0 && v_to_nb[v] > 1) {
                        //   If we detect a pinchout,
                        // update the number of crossed pinchout.
                        --nb_pinchouts;
                        // If we have crossed all the pinchouts,
                        // then we are again
                        // on the same border part that contains the initial
                        // segment.
                        in_hole_part = (nb_pinchouts == 0);
                    }
                    if(in_hole_part) {
                        clean_hole.push_back(hole[i]);
                    }
                }
                std::swap(hole,clean_hole);
            }


            index_t new_facet = mesh_grob()->facets.create_polygon(hole.size());
            for(index_t i=0; i<hole.size(); ++i) {
                index_t f = hole[i].facet;
                index_t c1 = hole[i].corner;
                index_t c2 = mesh_grob()->facets.next_corner_around_facet(f,c1);
                index_t v2 = mesh_grob()->facet_corners.vertex(c2);
                index_t new_c =
                    mesh_grob()->facets.corners_end(new_facet)-i-1;
                mesh_grob()->facet_corners.set_vertex(new_c,v2);
                mesh_grob()->facet_corners.set_adjacent_facet(
                    new_c,f
                );
                mesh_grob()->facet_corners.set_adjacent_facet(c1,new_facet);
            }

            mesh_grob()->update();
        }
    }

    /****************************************************************/

    void MeshGrobTransformFacet::pick_subset(
        MeshGrobTransformSubset* tool, const RayPick& rp
    ) {
        picked_facet_ = tool->pick_facet(rp);
        if(picked_facet_ == NO_FACET) {
            center_ = tool->picked_point();
        } else {
            center_ = Geom::mesh_facet_center(
                *tool->mesh_grob(), picked_facet_
            );
        }
    }

    void MeshGrobTransformFacet::transform_subset(const mat4& M) {
        if(picked_facet_ != NO_FACET) {
            for(
                index_t lv=0;
                lv<mesh_grob()->facets.nb_vertices(picked_facet_); ++lv
            ) {
                index_t v = mesh_grob()->facets.vertex(picked_facet_,lv);
                vec3& p = mesh_grob()->vertices.point(v);
                p = transform_point(p,M);
            }
        }
    }

    /****************************************************************/

    void MeshGrobJoinFacets::grab(const RayPick& p_ndc) {
        MeshGrobTool::grab(p_ndc);
        index_t picked_f = NO_FACET;
        index_t picked_c = NO_CORNER;
        if(!pick_facet_edge(p_ndc, picked_f, picked_c)) {
            Logger::err("Tool")
                << "No edge picked"
                << std::endl;
            return;
        }

        index_t adj_f = mesh_grob()->facet_corners.adjacent_facet(picked_c);
        if(adj_f == NO_FACET) {
            Logger::err("Tool")
                << "Picked edge is on the border"
                << std::endl;
            return;
        }

        if(adj_f == picked_f) {
            Logger::err("Tool")
                << "Facet is adjacent to itself along picked edge"
                << std::endl;
            return;
        }


        if(
            facet_has_bowtie(mesh_grob(), picked_f) ||
            facet_has_bowtie(mesh_grob(), adj_f)
        ) {
            Logger::err("Tool")
                << "Facet is incident to the same vertex more than once"
                << std::endl;
            return;
        }

        Loop loop;
        MeshHalfedges MH(*mesh_grob());
        MeshHalfedges::Halfedge firstH(picked_f, picked_c);

        MeshHalfedges::Halfedge H = firstH;
        while(mesh_grob()->facet_corners.adjacent_facet(H.corner) == adj_f) {
            MH.move_to_next_around_facet(H);
        }
        while(H != firstH) {
            if(mesh_grob()->facet_corners.adjacent_facet(H.corner) != adj_f) {
                loop.push_back(H);
            }
            MH.move_to_next_around_facet(H);
        }

        MH.move_to_opposite(firstH);
        H = firstH;
        while(mesh_grob()->facet_corners.adjacent_facet(H.corner) == picked_f) {
            MH.move_to_next_around_facet(H);
        }
        while(H != firstH) {
            if(
                mesh_grob()->facet_corners.adjacent_facet(H.corner) !=
                picked_f) {
                loop.push_back(H);
            }
            MH.move_to_next_around_facet(H);
        }

        create_facet_replace_loop(mesh_grob(),loop);
        vector<index_t> remove_f(mesh_grob()->facets.nb(), 0);
        remove_f[picked_f] = true;
        remove_f[adj_f] = true;
        mesh_grob()->facets.delete_elements(remove_f);
        mesh_grob()->update();
    }


    void MeshGrobSplitFacet::grab(const RayPick& p_ndc) {
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

        index_t picked_f = NO_FACET;
        index_t c1 = NO_CORNER;
        index_t c2 = NO_CORNER;
        for(index_t f=0; f<mesh_grob()->facets.nb(); ++f) {
            c1 = NO_CORNER;
            c2 = NO_CORNER;
            for(
                index_t c=mesh_grob()->facets.corners_begin(f);
                c < mesh_grob()->facets.corners_end(f); ++c
            ) {
                index_t v = mesh_grob()->facet_corners.vertex(c);
                if(v == v1_) {
                    c1 = c;
                } else if(v == v2_) {
                    c2 = c;
                }
            }
            if(c1 != NO_CORNER && c2 != NO_CORNER) {
                picked_f = f;
                break;
            }
        }

        if(picked_f == NO_FACET) {
            Logger::err("Tool")
                << "Did not find facet incident to both picked vertices"
                << std::endl;
            reset();
            return;
        }

        if(
            mesh_grob()->facets.next_corner_around_facet(picked_f, c1) == c2 ||
            mesh_grob()->facets.next_corner_around_facet(picked_f, c2) == c1
        ) {
            Logger::err("Tool")
                << "Facet edge already exists in mesh."
                << std::endl;
            reset();
            return;
        }

        index_t new_f[2];
        new_f[0] = NO_FACET;
        new_f[1] = NO_FACET;

        // We create the two new facets by turning around picked_f
        // first from c1 to c2, then from c2 to c1. This is done
        // with this two-iterations loop, that swaps c1 and c2 at
        // the end of the first iteration.

        for(index_t i=0; i<2; ++i) {
            index_t c2n =
                mesh_grob()->facets.next_corner_around_facet(picked_f, c2);

            index_t nb = 0;

            index_t c = c1;
            do {
                ++nb;
                c = mesh_grob()->facets.next_corner_around_facet(picked_f, c);
            } while(c != c2n);

            new_f[i] = mesh_grob()->facets.create_polygon(nb);

            // Create vertices
            c = c1;
            index_t new_c=mesh_grob()->facets.corners_begin(new_f[i]);
            do {
                index_t v = mesh_grob()->facet_corners.vertex(c);
                mesh_grob()->facet_corners.set_vertex(new_c, v);
                c = mesh_grob()->facets.next_corner_around_facet(picked_f, c);
                ++new_c;
            } while(c != c2n);

            // Connect new facets with old ones
            c = c1;
            new_c=mesh_grob()->facets.corners_begin(new_f[i]);
            do {
                index_t adj_f = mesh_grob()->facet_corners.adjacent_facet(c);
                connect_facets(mesh_grob(), new_f[i], new_c, adj_f);
                c = mesh_grob()->facets.next_corner_around_facet(picked_f, c);
                ++new_c;
            } while(c != c2);

            // Swap c1<->c2 then create the other facet (from c2 to c1 then)
	    std::swap(c1,c2);
        }


        // Connect the two newly created facets facets.
        //  The new edge is the one originated from the last
        // corner of the facet.
        connect_facets(
            mesh_grob(),
            new_f[0], mesh_grob()->facets.corners_end(new_f[0])-1,
            new_f[1], mesh_grob()->facets.corners_end(new_f[1])-1
        );

        vector<index_t> delete_f(mesh_grob()->facets.nb(), 0);
        delete_f[picked_f] = 1;
        mesh_grob()->facets.delete_elements(delete_f);

        mesh_grob()->update();

        reset();
    }

    void MeshGrobSplitFacet::reset() {
        v1_ = NO_VERTEX;
        v2_ = NO_VERTEX;
        MeshGrobTool::reset();
        Logger::out("Tool") << "Pick the first vertex" << std::endl;
    }

    void MeshGrobEditFacetEdge::reset() {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            object()->get_shader()
        );
        if(shd != nullptr) {
            shd->show_mesh();
            shd->show_vertices();
        }
        MultiTool::reset();
    }

    /************************************************************/

    MeshGrobEditHole::~MeshGrobEditHole() {
    }

}
