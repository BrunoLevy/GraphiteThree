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
 *  with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */


#include <OGF/mesh/commands/mesh_grob_volume_commands.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_surface_intersection.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/voronoi/RVD.h>
#include <geogram/voronoi/RVD_callback.h>
#include <geogram/voronoi/RVD_mesh_builder.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/progress.h>

#ifdef GEOGRAM_WITH_VORPALINE
#include <vorpalib/mesh/mesh_hexdom.h>
#include <vorpalib/mesh/mesh_refine.h>
#include <vorpalib/mesh/mesh_tet2hex.h>
#endif

namespace {
    using namespace OGF;

    class Histogram {
    public:
        Histogram(
            double min_bound, double max_bound, index_t nb_bins = 100
        ) :
            min_bound_(min_bound),
            max_bound_(max_bound),
            min_val_(Numeric::max_float64()),
            max_val_(Numeric::min_float64()),
            count_(nb_bins,0) {
        }
        void add_value(double val) {
            min_val_ = std::min(min_val_, val);
            max_val_ = std::max(max_val_, val);
            int i = int(
                double(count_.size()) *
                (val - min_bound_) / (max_bound_ - min_bound_)
            );
            geo_clamp(i,0,int(count_.size())-1);
            ++count_[i];
        }

        void save(const std::string& filename) {
            Logger::out("Histogram")
                << "Saving to file:"
                << filename << std::endl;
            std::ofstream out(filename.c_str());
            for(index_t i=0; i<count_.size(); ++i) {
                double x =
                    min_bound_ +
                    double(i)*(max_bound_ - min_bound_)/double(count_.size());
                out << x << " " << count_[i] << std::endl;
            }
        }

	/* // Unused for now
        double min_val() const {
            return min_val_;
        }

        double max_val() const {
            return min_val_;
        }
	*/

        std::string display_range() const {
            std::ostringstream os;
            os << "[" << min_val_ << "..." << max_val_ << "]" << std::ends;
            return os.str();
        }

    private:
        double min_bound_;
        double max_bound_;
        double min_val_;
        double max_val_;
        vector<index_t> count_;
    };

    void stat_cell_dihedral_angles(
        const MeshGrob& M, index_t c,
        Histogram& histo
    ) {
        geo_debug_assert(M.cells.nb_facets(c) <= 8);
        vec3 N[8];
        for(index_t lf=0; lf<M.cells.nb_facets(c); ++lf) {
            N[lf] = mesh_cell_facet_normal(M,c,lf);
        }
        for(index_t le=0; le<M.cells.nb_edges(c); ++le) {
            index_t lf1 = M.cells.edge_adjacent_facet(c,le,0);
            index_t lf2 = M.cells.edge_adjacent_facet(c,le,1);
            double angle = GEO::Geom::angle(N[lf1],N[lf2]);
            angle *= 180.0 / M_PI;
            histo.add_value(angle);
        }
    }

    void stat_cell_corner_angles(
        const MeshGrob& M, index_t c,
        Histogram& tri_histo,
        Histogram& quad_histo
    ) {
        for(index_t lf=0; lf<M.cells.nb_facets(c); ++lf) {
            index_t n = M.cells.facet_nb_vertices(c,lf);
            for(index_t i=0; i<n; ++i) {
                index_t j = (i+1)%n;
                index_t k = (j+1)%n;
                index_t vi = M.cells.facet_vertex(c,lf,i);
                index_t vj = M.cells.facet_vertex(c,lf,j);
                index_t vk = M.cells.facet_vertex(c,lf,k);
                const vec3& pi = GEO::Geom::mesh_vertex(M,vi);
                const vec3& pj = GEO::Geom::mesh_vertex(M,vj);
                const vec3& pk = GEO::Geom::mesh_vertex(M,vk);
                double alpha = GEO::Geom::angle(pi-pj, pk-pj) * 180.0 / M_PI;
                switch(n) {
                case 3: {
                    tri_histo.add_value(alpha);
                } break;
                case 4: {
                    quad_histo.add_value(alpha);
                } break;
                default:
                    ogf_assert_not_reached;
                }
            }
        }
    }


}

namespace OGF {

    MeshGrobVolumeCommands::MeshGrobVolumeCommands() {
    }

    MeshGrobVolumeCommands::~MeshGrobVolumeCommands() {
    }

    void MeshGrobVolumeCommands::tet_meshing(
        bool preprocess,
	bool merge_coplanar_facets,
        double epsilon, double max_hole_area,
        bool refine, double quality,
        bool keep_regions,
	bool verbose
    ) {
        CmdLine::set_arg("dbg:tetgen",verbose);
        mesh_grob()->cells.clear();
        mesh_grob()->vertices.remove_isolated();

        MeshTetrahedralizeParameters params;
        params.preprocess = preprocess;
	params.preprocess_merge_coplanar_facets = merge_coplanar_facets;
        params.preprocess_merge_vertices_epsilon = epsilon;
        params.preprocess_fill_hole_max_area = max_hole_area;
        params.refine = refine;
        params.refine_quality = quality;
        params.keep_regions = keep_regions;
        params.verbose = verbose;


        if(!mesh_tetrahedralize(*mesh_grob(), params)) {
            show_attribute("facets.selection");
            hide_vertices();
            mesh_grob()->update();
            return;
        }
        if(mesh_grob()->cells.nb() != 0) {
            mesh_grob()->cells.compute_borders();
        }
        show_mesh();
        mesh_grob()->update();
    }

    void MeshGrobVolumeCommands::hex_dominant_meshing(
        const NewMeshGrobName& hexdom_name,
        unsigned int nb_points,
        bool prisms,
        bool pyramids,
        bool border_refine,
        double border_max_dist,
        double min_normal_cos,
        double max_corner_cos
    ) {
#ifdef GEOGRAM_WITH_VORPALINE
        if(hexdom_name == mesh_grob()->name()) {
            Logger::err("HexDom") << "remesh should not be the same as mesh"
                                  << std::endl;
            return ;
        }
        if(!mesh_grob()->cells.are_simplices()) {
            Logger::err("HexDom") << "cells of input mesh should be only tets"
                                  << std::endl;
            return ;
        }
        if(mesh_grob()->cells.nb() == 0) {
            Logger::err("HexDom") << "Needs a tetrahedral mesh to operate"
                                  << std::endl;
            return;
        }

        MeshGrob* remesh = MeshGrob::find_or_create(
            scene_graph(), hexdom_name
        );
        remesh->clear();

        CmdLine::set_arg("hex:border_refine", border_refine);
        CmdLine::set_arg(
            "hex:border_max_distance",
            surface_average_edge_length(*mesh_grob()) * border_max_dist
        );

        mesh_hex_dominant(
            *mesh_grob(), *remesh,
            nb_points, prisms, pyramids,
            min_normal_cos, max_corner_cos
        );

        mesh_grob()->update();
        remesh->cells.connect();
        remesh->cells.compute_borders();
        remesh->update();

#else

        ogf_argused(hexdom_name);
        ogf_argused(nb_points);
        ogf_argused(prisms);
        ogf_argused(pyramids);
        ogf_argused(min_normal_cos);
        ogf_argused(max_corner_cos);
        ogf_argused(border_refine);
        ogf_argused(border_max_dist);

        Logger::err("HexDom") << "Needs Vorpaline/Vorpalib, contact authors"
                              << std::endl;
#endif
    }


/**********************************************************************/
}

namespace OGF {

    void MeshGrobVolumeCommands::Voronoi_meshing(
	const NewMeshGrobName& voronoi_name,
	index_t nb_points,
	VoronoiSimplification simplification,
	double angle_threshold,
	double shrink,
	const NewMeshGrobName& points_name,
	bool exact,
	bool tessellate_non_convex,
	bool generate_ids,
	bool medial_axis
    ) {

	generate_ids = generate_ids | medial_axis;

	if(!mesh_grob()->cells.are_simplices()) {
	    Logger::err("RVD") << "Mesh is not tetrahedral"
			       << std::endl;
	    return;
	}

	if(mesh_grob()->cells.nb() == 0) {
	    mesh_tetrahedralize(*mesh_grob());
	}


	MeshGrob* points = nullptr;

	if(points_name !="") {
	    points = MeshGrob::find(
		scene_graph(), points_name
	    );

	    if(points == nullptr || points->vertices.nb() == 0) {
		Logger::err("RVD") << "Did not find points"
				   << std::endl;
		return;
	    }
	}

        MeshGrob* voronoi = MeshGrob::find_or_create(
            scene_graph(), voronoi_name
        );
        voronoi->clear();


        CentroidalVoronoiTesselation CVT(mesh_grob());
	CVT.set_volumetric(!medial_axis);
	Delaunay* delaunay = CVT.delaunay();
	RestrictedVoronoiDiagram* RVD = CVT.RVD();

	if(points == nullptr) {
	    CVT.compute_initial_sampling(nb_points);

	    try {
		ProgressTask progress("Lloyd", 100);
		CVT.set_progress_logger(&progress);
		CVT.Lloyd_iterations(5);
	    }
	    catch(const TaskCanceled&) {
	    }

	    try {
		ProgressTask progress("Newton", 100);
		CVT.set_progress_logger(&progress);
		CVT.Newton_iterations(30, 7);
	    }
	    catch(const TaskCanceled&) {
	    }
	} else {
	    delaunay->set_vertices(
		points->vertices.nb(), points->vertices.point_ptr(0)
	    );
	}

	RVD->set_exact_predicates(exact);

	{
	    BuildRVDMesh callback(*voronoi);
	    switch(simplification) {
		case keep_everything:
		    callback.set_simplify_internal_tet_facets(false);
		    callback.set_simplify_voronoi_facets(false);
		    break;
		case simplify_tet:
		    callback.set_simplify_internal_tet_facets(true);
		    callback.set_simplify_voronoi_facets(false);
		    break;
		case simplify_tet_voro:
		    callback.set_simplify_internal_tet_facets(true);
		    callback.set_simplify_voronoi_facets(true);
		    break;
	    }
	    if(angle_threshold != 0.0) {
		callback.set_simplify_boundary_facets(true,angle_threshold);
	    }
	    callback.set_shrink(shrink);
	    callback.set_tessellate_non_convex_facets(tessellate_non_convex);
	    callback.set_generate_ids(generate_ids);
	    RVD->for_each_polyhedron(callback);
	}
	voronoi->update();

	if(medial_axis) {
	    vector<index_t> to_delete(voronoi->facets.nb(),0);
	    Attribute<signed_index_t> facet_seed(voronoi->facets.attributes(),"facet_seed_id");
	    for(index_t f: voronoi->facets) {
		if(facet_seed[f] == -1) {
		    to_delete[f] = 1;
		    for(index_t le=0; le<voronoi->facets.nb_vertices(f); ++le) {
			index_t adj = voronoi->facets.adjacent(f,le);
			if(adj != NO_FACET) {
			    to_delete[adj] = 1;
			}
		    }
		}
	    }
	    voronoi->facets.delete_elements(to_delete);
	    double epsilon = 1e-6 * (0.01 * bbox_diagonal(*mesh_grob()));
	    mesh_repair(
		*voronoi,
		GEO::MeshRepairMode(
		    GEO::MESH_REPAIR_COLOCATE | GEO::MESH_REPAIR_DUP_F
		),
		epsilon
	    );
	    voronoi->update();
	}
    }

/**********************************************************************/

    void MeshGrobVolumeCommands::volume_mesh_statistics(
        bool save_histo, index_t nb_bins
    ) {
        double total_volume = mesh_cells_volume(*mesh_grob());
        double hex_volume = 0.0;
        index_t nb_cells = 0;
        index_t nb_hex = 0;

        Histogram dihedral_angle_hex(0.0, 180.0, nb_bins);
        Histogram dihedral_angle_other(0.0, 180.0, nb_bins);
        Histogram corner_angle_tri(0.0, 180.0, nb_bins);
        Histogram corner_angle_quad(0.0, 180.0, nb_bins);

        for(index_t c: mesh_grob()->cells) {
            MeshCellType type = mesh_grob()->cells.type(c);
            if(type == MESH_CONNECTOR) {
                continue;
            }
            ++nb_cells;
            if(type == MESH_HEX) {
                ++nb_hex;
                hex_volume += mesh_cell_volume(*mesh_grob(),c);
            }
        }


        for(index_t c: mesh_grob()->cells) {
            MeshCellType type = mesh_grob()->cells.type(c);
            if(type == MESH_CONNECTOR) {
                continue;
            }
            if(type == MESH_HEX) {
                stat_cell_dihedral_angles(
                    *mesh_grob(), c, dihedral_angle_hex
                );
            } else {
                stat_cell_dihedral_angles(
                    *mesh_grob(), c, dihedral_angle_other
                );
            }
        }

        for(index_t c: mesh_grob()->cells) {
            MeshCellType type = mesh_grob()->cells.type(c);
            if(type == MESH_CONNECTOR) {
                continue;
            }
            stat_cell_corner_angles(
                *mesh_grob(), c,
                corner_angle_tri,
                corner_angle_quad
            );
        }

        if(total_volume == 0.0 || nb_cells == 0) {
            Logger::warn("Stats")
                << "Mesh does not have any cell and/or zero volume"
                << std::endl;
            return;
        }
        Logger::out("Stats") << "Nb hexes "
                             << nb_hex
                             << " / Total nb cells "
                             << nb_cells
                             << std::endl;
        Logger::out("Stats") << "Proportion nb hexes "
                             << 100.0 * double(nb_hex) / double(nb_cells)
                             << "%"
                             << std::endl;
        Logger::out("Stats") << "Proportion vol hexes "
                             << 100.0 * hex_volume / total_volume
                             << std::endl;
        Logger::out("Stats") << "Angles (trgls) "
                             << corner_angle_tri.display_range()
                             << std::endl;
        Logger::out("Stats") << "Angles (quads) "
                             << corner_angle_quad.display_range()
                             << std::endl;
        Logger::out("Stats") << "Angles (hexes) "
                             << dihedral_angle_hex.display_range()
                             << std::endl;
        Logger::out("Stats") << "Angles (cells) "
                             << dihedral_angle_other.display_range()
                             << std::endl;

        if(save_histo) {
            dihedral_angle_hex.save("dihedral_angle_hex.dat");
            dihedral_angle_other.save("dihedral_angle_other.dat");
            corner_angle_tri.save("corner_angle_tri.dat");
            corner_angle_quad.save("corner_angle_quad.dat");
        }
    }


    void MeshGrobVolumeCommands::tet_meshing_with_points(
        const MeshGrobName& points_name,
        const NewMeshGrobName& tetrahedra_name,
        bool refine_surface,
        double max_distance
    ) {

#ifndef GEOGRAM_WITH_VORPALINE
        geo_argused(max_distance);
        if(refine_surface) {
            Logger::warn("TetMesh")
                << "refine_surface only supported with Vorpaline" << std::endl;
            Logger::warn("TetMesh")
                << "(deactivated in this version, contact authors if needed)"
                << std::endl;
            refine_surface = false;
        }
#endif

        MeshGrob* points = MeshGrob::find(scene_graph(), points_name);
        if(points == nullptr) {
            Logger::err("TetMesh") << points_name << ": no such point set"
                                   << std::endl;
            return;
        }

        MeshGrob* tets = MeshGrob::find_or_create(
            scene_graph(), tetrahedra_name
        );
        tets->clear();

        std::string predicates_mode_bkp = CmdLine::get_arg("algo:predicates");
        CmdLine::set_arg("algo:predicates","exact");

        Mesh surface;
        vector<double> inner_points;

        {
            Logger::div("Computing the surface");

            Delaunay_var delaunay = Delaunay::create(3);
            RestrictedVoronoiDiagram_var RVD =
                RestrictedVoronoiDiagram::create(delaunay,mesh_grob());


            RestrictedVoronoiDiagram::RDTMode mode =
                RestrictedVoronoiDiagram::RDTMode(
                    RestrictedVoronoiDiagram::RDT_MULTINERVE     |
                    RestrictedVoronoiDiagram::RDT_RVC_CENTROIDS  |
                    RestrictedVoronoiDiagram::RDT_PREFER_SEEDS   |
                    RestrictedVoronoiDiagram::RDT_SELECT_NEAREST
                );


            if(refine_surface) {
                mode = RestrictedVoronoiDiagram::RDTMode(
                    RestrictedVoronoiDiagram::RDT_SEEDS_ALWAYS |
                    RestrictedVoronoiDiagram::RDT_DONT_REPAIR
                );
            } else {
                // RDT_PREFER_SEEDS needs the surface mesh to be reordered
                // since it uses a MeshFacetsAABB to determine whether the
                // seed or the centroid of the connected component of the
                // RVD cell should be used.
                mesh_reorder(*mesh_grob(), MESH_ORDER_MORTON);
                mesh_grob()->update();
            }

            delaunay->set_vertices(
                points->vertices.nb(), points->vertices.point_ptr(0)
            );

            RVD->compute_RDT(surface, mode);

#ifdef GEOGRAM_WITH_VORPALINE
            if(refine_surface) {
                max_distance *= surface_average_edge_length(*mesh_grob());
                mesh_refine(surface, *mesh_grob(), max_distance);
            }
#endif
            mesh_repair(surface);
            remove_small_connected_components(surface,0.0,100);
            fill_holes(surface, 1e30);
            double radius = bbox_diagonal(surface);
            remove_degree3_vertices(surface, 0.001*radius);
            mesh_remove_intersections(surface);
            mesh_repair(surface);

            vector<double> m(points->vertices.nb());
            vector<double> mg(points->vertices.nb()*3);
            RVD->compute_centroids_on_surface(mg.data(), m.data());
            for(index_t v: points->vertices) {
                if(m[v] == 0.0) {
                    inner_points.push_back(
                        points->vertices.point_ptr(v)[0]
                    );
                    inner_points.push_back(
                        points->vertices.point_ptr(v)[1]
                    );
                    inner_points.push_back(
                        points->vertices.point_ptr(v)[2]
                    );
                }
            }
        }

        {
            Logger::div("Constrained triangulation");
            Delaunay_var delaunay = Delaunay::create(3,"tetgen");
            delaunay->set_constraints(&surface);
            delaunay->set_vertices(inner_points.size()/3, inner_points.data());

            vector<double> pts(delaunay->nb_vertices() * 3);
            vector<index_t> tet2v(delaunay->nb_cells() * 4);
            for(index_t v = 0; v < delaunay->nb_vertices(); ++v) {
                pts[3 * v] = delaunay->vertex_ptr(v)[0];
                pts[3 * v + 1] = delaunay->vertex_ptr(v)[1];
                pts[3 * v + 2] = delaunay->vertex_ptr(v)[2];
            }
            for(index_t t = 0; t < delaunay->nb_cells(); ++t) {
                tet2v[4 * t] = index_t(delaunay->cell_vertex(t, 0));
                tet2v[4 * t + 1] = index_t(delaunay->cell_vertex(t, 1));
                tet2v[4 * t + 2] = index_t(delaunay->cell_vertex(t, 2));
                tet2v[4 * t + 3] = index_t(delaunay->cell_vertex(t, 3));
            }
            tets->cells.assign_tet_mesh(3, pts, tet2v, true);
            tets->cells.connect();
            tets->cells.compute_borders();
            tets->update();
            tets->show_stats("TetMeshing");
        }
        CmdLine::set_arg("algo:predicates",predicates_mode_bkp);
    }

    void MeshGrobVolumeCommands::tet2hex(
        const NewMeshGrobName& hexdom_name,
        bool prisms,
        bool pyramids,
        double min_normal_cos,
        double max_corner_cos
    ) {
#ifdef GEOGRAM_WITH_VORPALINE
        if(hexdom_name == mesh_grob()->name()) {
            Logger::err("HexDom") << "remesh should not be the same as mesh"
                                  << std::endl;
            return ;
        }
        if(!mesh_grob()->cells.are_simplices()) {
            Logger::err("HexDom") << "cells of input mesh should be only tets"
                                  << std::endl;
            return ;
        }
        if(mesh_grob()->cells.nb() == 0) {
            Logger::err("HexDom") << "Needs a tetrahedral mesh to operate"
                                  << std::endl;
            return;
        }

        MeshGrob* remesh = MeshGrob::find_or_create(
            scene_graph(), hexdom_name
        );
        remesh->clear();

        mesh_tet2hex(
            *mesh_grob(), *remesh,
            min_normal_cos, max_corner_cos,
            prisms, pyramids
        );

        mesh_grob()->update();
        remesh->cells.connect();
        remesh->cells.compute_borders();
        remesh->update();
#else
        ogf_argused(hexdom_name);
        ogf_argused(prisms);
        ogf_argused(pyramids);
        ogf_argused(min_normal_cos);
        ogf_argused(max_corner_cos);

        Logger::err("HexDom") << "Needs Vorpaline/Vorpalib, contact authors"
                              << std::endl;
#endif
    }


    void MeshGrobVolumeCommands::remesh_tetrahedra(double quality) {

        // Copy tetrahedra from input mesh

        Mesh constraints;
        constraints.copy(
            *mesh_grob(),
            false,
            MeshElementsFlags(MESH_VERTICES | MESH_CELLS)
        );

        vector<index_t> kill_cell(constraints.cells.nb(),0);
        for(index_t cell: constraints.cells) {
            if(constraints.cells.type(cell) != MESH_TET) {
                kill_cell[cell] = true;
            }
        }

        // Extract the border of the tetrahedra
        constraints.cells.delete_elements(kill_cell, true);
        constraints.cells.compute_borders();
        constraints.cells.clear();
        constraints.vertices.remove_isolated();

        // Re-tetrahedralize the surface
        Delaunay_var delaunay = Delaunay::create(3,"tetgen");
        delaunay->set_quality(quality);
        delaunay->set_refine(true);
        delaunay->set_constraints(&constraints);
        delaunay->set_vertices(0,nullptr);

        // Replace the original tetrahedra with the new ones.

        mesh_grob()->facets.clear();
        mesh_grob()->vertices.remove_isolated();

        kill_cell.assign(mesh_grob()->cells.nb(), 0);
        for(index_t cell: mesh_grob()->cells) {
            if(mesh_grob()->cells.type(cell) == MESH_TET) {
                kill_cell[cell] = true;
            }
        }
        mesh_grob()->cells.delete_elements(kill_cell, true);

        index_t v_offset =
            mesh_grob()->vertices.create_vertices(delaunay->nb_vertices());

        for(index_t v=0; v<delaunay->nb_vertices(); ++v) {
            const double* from = delaunay->vertex_ptr(v);
            double* to = mesh_grob()->vertices.point_ptr(v_offset+v);
            to[0] = from[0];
            to[1] = from[1];
            to[2] = from[2];
        }

        index_t t_offset =
            mesh_grob()->cells.create_tets(delaunay->nb_cells());

        for(index_t t=0; t<delaunay->nb_cells(); ++t) {
            for(index_t lv=0; lv<4; ++lv) {
                mesh_grob()->cells.set_vertex(
                    t_offset + t, lv,
                    v_offset + index_t(delaunay->cell_vertex(t, lv))
                );
            }
        }

        mesh_repair(*mesh_grob(), MESH_REPAIR_COLOCATE);

        //    Eliminate the new tetrahedra that are slivers glued
        // onto a quad face.

        kill_cell.assign(mesh_grob()->cells.nb(), 0);

        std::set<quadindex> quad_faces;
        Logger::out("Mesh") << "Finding quad faces" << std::endl;
        for(index_t cell: mesh_grob()->cells) {
            if(mesh_grob()->cells.type(cell) == MESH_HEX) {
                for(index_t lf=0; lf<6; ++lf) {
                    index_t v1 = mesh_grob()->cells.facet_vertex(cell,lf,0);
                    index_t v2 = mesh_grob()->cells.facet_vertex(cell,lf,1);
                    index_t v3 = mesh_grob()->cells.facet_vertex(cell,lf,2);
                    index_t v4 = mesh_grob()->cells.facet_vertex(cell,lf,3);
                    quad_faces.insert(quadindex(v1,v2,v3,v4));
                }
            }
        }

        Logger::out("Mesh") << "Removing slivers" << std::endl;
        index_t nb_slivers = 0;
        for(index_t cell: mesh_grob()->cells) {
            if(mesh_grob()->cells.type(cell) == MESH_TET) {
                index_t v1 = mesh_grob()->cells.vertex(cell,0);
                index_t v2 = mesh_grob()->cells.vertex(cell,1);
                index_t v3 = mesh_grob()->cells.vertex(cell,2);
                index_t v4 = mesh_grob()->cells.vertex(cell,3);
                quadindex K(v1,v2,v3,v4);
                if(quad_faces.find(K) != quad_faces.end()) {
                    kill_cell[cell] = 1;
                    ++nb_slivers;
                }
            }
        }

        if(nb_slivers != 0) {
            mesh_grob()->cells.delete_elements(kill_cell, false);
            Logger::out("Mesh") << "Deleted " << nb_slivers << " sliver(s)"
                                << std::endl;
        } else {
            Logger::out("Mesh") << "Did not find any new sliver"
                                << std::endl;
        }

        mesh_grob()->cells.connect();
        mesh_grob()->cells.compute_borders();
        mesh_grob()->facets.connect();

        mesh_grob()->update();

    }

    void MeshGrobVolumeCommands::display_volume() {
	Logger::out("Mesh") << "Cells volume    = "
			    << mesh_cells_volume(*mesh_grob()) << std::endl;
	Logger::out("Mesh") << "Enclosed volume = "
			    << Geom::mesh_enclosed_volume(*mesh_grob())
			    << std::endl;
	Logger::out("Mesh") << "Area = "
			    << Geom::mesh_area(*mesh_grob())
			    << std::endl;
    }

    void MeshGrobVolumeCommands::compute_borders() {
	mesh_grob()->cells.compute_borders();
	mesh_grob()->update();
    }

}
