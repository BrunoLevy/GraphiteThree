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
 * As an exception to the GPL, Graphite can be linked with the
 *  following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

#include <OGF/WarpDrive/commands/mesh_grob_transport_commands.h>
#include <OGF/WarpDrive/algo/VSDM.h>

#define READ_HYDRA_LIB_ONLY
#include <OGF/WarpDrive/IO/read_hydra.h>

#include <exploragram/optimal_transport/sampling.h>
#include <exploragram/optimal_transport/optimal_transport_3d.h>
#include <exploragram/optimal_transport/optimal_transport_2d.h>
#include <exploragram/optimal_transport/optimal_transport_on_surface.h>

#include <OGF/scene_graph/types/scene_graph.h>

#include <geogram/basic/command_line.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh_sampling.h>
#include <geogram/mesh/mesh_subdivision.h>
#include <geogram/voronoi/RVD_callback.h>
#include <geogram/voronoi/generic_RVD_cell.h>
#include <geogram/delaunay/CDT_2d.h>
#include <geogram/points/kd_tree.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/progress.h>
#include <geogram/basic/permutation.h>
#include <geogram/basic/line_stream.h>

// We got some classes declared locally that
// have no out-of-line virtual functions. It is not a
// problem since they are only visible from this translation
// unit, but clang will complain.
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

namespace {
    using namespace OGF;

    /**
     * \brief Appends a surface mesh to this mesh.
     * \param[in,out] to a pointer to a mesh
     * \param[in] comp the mesh to be appended to \p to
     */
    void append_surface_mesh(Mesh* to, Mesh* comp) {
        index_t offset = to->vertices.nb();
        for(index_t v=0; v<comp->vertices.nb(); ++v) {
            to->vertices.create_vertex(comp->vertices.point_ptr(v));
        }
        for(index_t f=0; f<comp->facets.nb(); ++f) {
            index_t nf = to->facets.create_polygon(comp->facets.nb_vertices(f));
            for(index_t lv=0; lv<comp->facets.nb_vertices(f); ++lv) {
                to->facets.set_vertex(
                    nf,lv,comp->facets.vertex(f,lv) + offset
                );
            }
        }
    }

    /**
     * \brief Appends a volume mesh to this mesh.
     * \param[in,out] to a pointer to a mesh
     * \param[in] comp the mesh to be appended to \p to
     */
    void append_volume_mesh(Mesh* to, Mesh* comp) {
        geo_assert(comp->cells.are_simplices());
        index_t v_offset = to->vertices.nb();
        for(index_t v=0; v<comp->vertices.nb(); ++v) {
            to->vertices.create_vertex(comp->vertices.point_ptr(v));
        }
        index_t t_offset = to->cells.create_tets(comp->cells.nb());
        for(index_t t=0; t<comp->cells.nb(); ++t) {
            for(index_t lv=0; lv<4; ++lv) {
                to->cells.set_vertex(
                    t + t_offset,lv,comp->cells.vertex(t,lv) + v_offset
                );
            }
        }
    }
}

namespace OGF {

    MeshGrobTransportCommands::MeshGrobTransportCommands() {
    }

    MeshGrobTransportCommands::~MeshGrobTransportCommands() {
    }

    void MeshGrobTransportCommands::align(
        const MeshGrobName& target_name,
        bool recenter,
        bool rescale
    ) {
        if(mesh_grob()->cells.nb() == 0) {
            Logger::err("WarpDrive") << " this mesh has no tetrahedron"
                                     << std::endl;
            return;
        }

        MeshGrob* target = MeshGrob::find(scene_graph(),target_name);
        if(target == nullptr) {
            Logger::err("WarpDrive") << target_name << ": no such MeshGrob"
                                     << std::endl;
            return;
        }

        if(target->cells.nb() == 0) {
            Logger::err("WarpDrive") << target_name << " has no tetrahedron"
                                     << std::endl;
            return;
        }

        if(recenter) {
            recenter_mesh(*target, *mesh_grob());
            mesh_grob()->update();
        }

        if(rescale) {
            rescale_mesh(*target, *mesh_grob());
            mesh_grob()->update();
        }
    }

    void MeshGrobTransportCommands::set_density_by_formula(
        double density1,
        double density2,
        const std::string& function,
        const std::string& reference_name
    ) {
        if(mesh_grob()->cells.nb() == 0) {
            Logger::err("WarpDrive") << " this mesh has no tetrahedron"
                                     << std::endl;
            return;
        }
        MeshGrob* reference = nullptr;
        if(reference_name != "") {
            reference = MeshGrob::find(scene_graph(),reference_name);
            if(reference == nullptr) {
                Logger::err("WarpDrive")
                    << reference_name << ": no such MeshGrob"
                    << std::endl;
                return;
            }
        }
        ::OGF::set_density(
            *mesh_grob(), density1, density2, function, reference
        );
        if(reference != nullptr) {
            reference->update();
        }
        mesh_grob()->update();
    }

    void MeshGrobTransportCommands::DESI_normalize_and_compute_selection_function(
        index_t nb_bins, index_t subsample
    ) {
        if(subsample != 0) {
            vector<GEO::index_t> indices(mesh_grob()->vertices.nb());
            for(index_t i: mesh_grob()->vertices) {
                indices[i] = i;
            }
            std::random_shuffle(indices.begin(), indices.end());
            Permutation::apply(mesh_grob()->vertices.point_ptr(0), indices, sizeof(double)*3);
            for(index_t i : mesh_grob()->vertices) {
                indices[i] = (i >= subsample) ? 1 : 0;
            }
            mesh_grob()->vertices.delete_elements(indices);
        }

        double max_radius = 0.0;
        for(index_t v: mesh_grob()->vertices) {
            vec3 p(mesh_grob()->vertices.point_ptr(v));
            max_radius = std::max(max_radius,length2(p));
        }
        max_radius = ::sqrt(max_radius);
        double s = 0.5 / max_radius;

        // Normalize in [0,1]^3 cube
        for(index_t v: mesh_grob()->vertices) {
            vec3 p(mesh_grob()->vertices.point_ptr(v));
            p *= s;
            mesh_grob()->vertices.point_ptr(v)[0] = 0.5 + p.x;
            mesh_grob()->vertices.point_ptr(v)[1] = 0.5 + p.y;
            mesh_grob()->vertices.point_ptr(v)[2] = 0.5 + p.z;
        }

        // Compute histogram
        vector<index_t> histo(nb_bins);
        vec3 C(0.5, 0.5, 0.5);
        for(index_t v: mesh_grob()->vertices) {
            vec3 p(mesh_grob()->vertices.point_ptr(v));
            p -= C;
            double R = 2.0*length(p);
            index_t i = index_t(R * double(nb_bins));
            i = std::min(i, nb_bins-1);
            ++histo[i];
        }

        for(index_t i=0; i<histo.size(); ++i) {
            std::cerr << i << ":" << histo[i] << std::endl;
        }

        Attribute<double> density(
            mesh_grob()->vertices.attributes(), "mass"
        );

        for(index_t v: mesh_grob()->vertices) {
            vec3 p(mesh_grob()->vertices.point_ptr(v));
            p -= C;
            double R = 2.0*length(p);
            index_t i = index_t(R * double(nb_bins));
            i = std::min(i, nb_bins-1);
            density[v] = 1.0/double(histo[i]);
        }

        mesh_grob()->update();
    }

    void MeshGrobTransportCommands::transport_3d(
        const MeshGrobName& target_name,
        index_t nb_points,
        bool project_on_border,
        const NewMeshGrobName& result_name,
        bool Newton,
        const NewMeshGrobName& sampling_name,
        const NewMeshGrobName& singular_set_name,
        double regularization,
        bool BRIO,
        bool multilevel,
        double ratio,
        index_t nb_iter,
        double epsilon,
	index_t linsolve_nb_iter,
	double linsolve_epsilon,
        bool save_RVD_iter,
        bool show_RVD_centers,
        bool save_last_iter
    ) {

	if(target_name == mesh_grob()->name()) {
	    Logger::err("Transport")
		<< "Transporting object to itselt will not be very interesting"
		<< std::endl;
	    return;
	}

        //  For now, hierarchical mode does not work with Newton, because
        // when we refine, there can be empty cells...
        if(Newton) {
            BRIO = false;
            multilevel = false;
        }


        if(mesh_grob()->cells.nb() == 0) {
            Logger::err("WarpDrive") << " this mesh has no tetrahedron"
                                     << std::endl;
            return;
        }

        MeshGrob* target = MeshGrob::find(scene_graph(),target_name);
        if(target == nullptr) {
            Logger::err("WarpDrive") << target_name << ": no such MeshGrob"
                                     << std::endl;
            return;
        }

        if(target->cells.nb() == 0) {
            Logger::err("WarpDrive") << target_name << " has no tetrahedron"
                                     << std::endl;
            return;
        }

        // Step 1: Sample

        Mesh points;
        vector<index_t> levels;

        CentroidalVoronoiTesselation CVT(target, 0, "NN");
	CVT.set_volumetric(true);
        if(nb_points != 0) {
            sample(
                CVT, nb_points, project_on_border, BRIO,
                multilevel, ratio, &levels
            );
            points.vertices.assign_points(
                CVT.embedding(0), CVT.dimension(), CVT.nb_points()
            );
        }

        if(sampling_name != "") {
            // Copy the samples if the user wants to see them...
            MeshGrob* sampling = MeshGrob::find_or_create(
                scene_graph(), sampling_name
            );

            if(nb_points != 0) {
                sampling->clear();
                sampling->vertices.assign_points(
                    points.vertices.point_ptr(0),
                    points.vertices.dimension(), points.vertices.nb()
                );
                sampling->update();
            } else {
		sampling->vertices.set_dimension(3);
		if(BRIO || multilevel) {
		    vector<index_t> sorted_indices;
		    compute_BRIO_order(
			sampling->vertices.nb(),
			sampling->vertices.point_ptr(0), sorted_indices,
			3, // Dim
			sampling->vertices.dimension(), // Stride
		        300, ratio, &levels
		    );
		    Permutation::apply(
			sampling->vertices.point_ptr(0),
			sorted_indices,
			index_t(sampling->vertices.dimension() * sizeof(double))
		    );
		}
		sampling->update();

                nb_points = sampling->vertices.nb();
                points.vertices.assign_points(
                    sampling->vertices.point_ptr(0),
                    sampling->vertices.dimension(),
                    sampling->vertices.nb()
                );

                CVT.set_points(
                    sampling->vertices.nb(),
                    sampling->vertices.point_ptr(0)
                );

                //  It is too stupid, set_points() does not
                // do that, I probably need to change it...
                CVT.delaunay()->set_vertices(
                    sampling->vertices.nb(),
                    sampling->vertices.point_ptr(0)
                );
                CVT.set_volumetric(true);
            }

            // TODO: have a function for that (duplicated
            // in MeshGrobTool)
            std::string vertices_prop;
            if(
                sampling->get_shader()->get_property(
                    "vertices_style", vertices_prop
                )
            ) {
                size_t pos = vertices_prop.find(';');
                ogf_assert(pos != std::string::npos);
                vertices_prop = "true;" + vertices_prop.substr(
                    pos+1, vertices_prop.length()-pos-1
                    );
                sampling->get_shader()->set_property(
                    "vertices_style", vertices_prop
                    );
                sampling->save("transport_sampling.geogram");
            }
        }

        if(nb_points == 0) {
            Logger::err("Transport")
                << "If nb_points is set to zero, we need a non-empty sampling"
                << std::endl;
            return;
        }


        // Step 2: Transport

        // Everything happens in dimension 4 (power diagram is seen
        // as Voronoi diagram in dimension 4), therefore the dimension
        // of M1 needs to be changed as well (even if it is not used).
        mesh_grob()->vertices.set_dimension(4);
        mesh_grob()->update();

        if(save_RVD_iter) {
            scene_graph()->disable_signals();
        }

        OptimalTransportMap3d OTM(mesh_grob(), "default", BRIO);

        if(Newton) {
            regularization = std::max(regularization, 1e-3);
            OTM.set_Newton(true);
        }

        OTM.set_regularization(regularization);

        OTM.set_save_RVD_iter(
            save_RVD_iter, show_RVD_centers, save_last_iter
        );

        OTM.set_points(
            points.vertices.nb(), points.vertices.point_ptr(0)
        );
        OTM.set_epsilon(epsilon);
	OTM.set_linsolve_maxiter(linsolve_nb_iter);
	OTM.set_linsolve_epsilon(linsolve_epsilon);
        {
            Stopwatch W("OTM Total");
            if(
                levels.size() > 0 &&
                levels[levels.size()-1] == points.vertices.nb()
            ) {
                Logger::out("OTM") << "Using multilevel algorithm" << std::endl;
                OTM.optimize_levels(levels, nb_iter);
            } else {
                OTM.optimize(nb_iter);
            }
        }

        if(save_RVD_iter) {
            scene_graph()->enable_signals();
            scene_graph()->update();
        }

        // Step 3: extract result
        if(result_name != "") {
            MeshGrob* result = MeshGrob::find_or_create(
                scene_graph(),result_name
            );
            result->clear();
            result->vertices.set_dimension(6);

            Logger::div("Morphing");
            Logger::out("OTM") <<  "Time-coherent triangulation." << std::endl;

            bool filter_tets = (target != mesh_grob());
            compute_morph(CVT, OTM, *result, filter_tets);
            result->update();
            result->get_shader()->set_property("animate","true");
        }


        if(singular_set_name != "") {
            MeshGrob* singular_set =
                MeshGrob::find_or_create(scene_graph(),singular_set_name);
            singular_set->clear();
            singular_set->vertices.set_dimension(3);
            Logger::out("OTM") << "Computing singular set" << std::endl;
            compute_singular_surface(CVT,OTM, *singular_set);
            singular_set->update();
        }

        if(sampling_name != "") {
            Logger::out("OTM") << "Computing animation for the points"
                               << std::endl;
            // Animate the points
            MeshGrob* sampling = MeshGrob::find(
                scene_graph(), sampling_name
            );
            vector<double> centroids(sampling->vertices.nb()*3);
            OTM.compute_Laguerre_centroids(centroids.data());
            sampling->vertices.set_dimension(6);
            for(index_t v=0; v<sampling->vertices.nb(); ++v) {
                for(index_t c=0; c<3; ++c) {
                    sampling->vertices.point_ptr(v)[3+c] = centroids[3*v+c];
                }
            }
        }

        mesh_grob()->vertices.set_dimension(3);
        mesh_grob()->update();
    }

    void MeshGrobTransportCommands::early_universe_reconstruction(
	const MeshGrobName& points_name,
	double scaling_factor,
	double Newton_epsilon,
	index_t max_Newton_iter,
	index_t linesearch_init_iter,
	index_t max_linesearch_iter,
	double linsolve_epsilon,
	index_t max_linsolve_iter,
	double regularization
    ) {
        if(mesh_grob()->cells.nb() == 0) {
            Logger::err("WarpDrive") << " this mesh has no tetrahedron"
                                     << std::endl;
            return;
        }

	MeshGrob* points = MeshGrob::find(scene_graph(), points_name);

	if(points == nullptr) {
            Logger::err("WarpDrive") << points_name
				     << " no such pointset"
                                     << std::endl;
	    return;
	}

	if(points->vertices.nb() == 0) {
            Logger::err("WarpDrive") << points_name
				     << " has no point"
                                     << std::endl;
	    return;
	}

	// Remove previous animation if present.
	points->vertices.set_dimension(3);
	points->update();

	// Apply scaling
	FOR(i,points->vertices.nb()*points->vertices.dimension()) {
	    points->vertices.point_ptr(0)[i] *= scaling_factor;
	}

	FOR(i,mesh_grob()->vertices.nb()*mesh_grob()->vertices.dimension()) {
	    mesh_grob()->vertices.point_ptr(0)[i] *= scaling_factor;
	}

        // Step 2: Transport
        // Everything happens in dimension 4 (power diagram is seen
        // as Voronoi diagram in dimension 4), therefore the dimension
        // of M1 needs to be changed as well (even if it is not used).
        mesh_grob()->vertices.set_dimension(4);
        mesh_grob()->update();

        OptimalTransportMap3d OTM(mesh_grob(), "default");

	regularization = std::max(regularization, 1e-3);
	OTM.set_Newton(true);
        OTM.set_regularization(regularization);
        OTM.set_points(points->vertices.nb(), points->vertices.point_ptr(0));
        OTM.set_epsilon(Newton_epsilon);
	OTM.set_linsolve_maxiter(max_linsolve_iter);
	OTM.set_linsolve_epsilon(linsolve_epsilon);
	OTM.set_linesearch_init_iter(linesearch_init_iter);
	OTM.set_linesearch_maxiter(max_linesearch_iter);
        {
            Stopwatch W("OTM Total");
	    OTM.optimize(max_Newton_iter);
        }


	{
            Logger::out("OTM") << "Computing animation for the points"
                               << std::endl;
            vector<double> centroids(points->vertices.nb()*3);
            OTM.compute_Laguerre_centroids(centroids.data());
            points->vertices.set_dimension(6);
            for(index_t v=0; v<points->vertices.nb(); ++v) {
                for(index_t c=0; c<3; ++c) {
                    points->vertices.point_ptr(v)[3+c] = centroids[3*v+c];
                }
            }
	    points->update();
        }

        mesh_grob()->vertices.set_dimension(3);

	// "un-apply" scaling.

	FOR(i,points->vertices.nb()*points->vertices.dimension()) {
	    points->vertices.point_ptr(0)[i] /= scaling_factor;
	}

	FOR(i,mesh_grob()->vertices.nb()*mesh_grob()->vertices.dimension()) {
	    mesh_grob()->vertices.point_ptr(0)[i] /= scaling_factor;
	}


        mesh_grob()->update();
    }


    void MeshGrobTransportCommands::create_regular_sampling(
        const NewMeshGrobName& sampling_name,
        index_t nb
    ) {

        Box box = mesh_grob()->bbox();
        double l = 0.0;
        for(index_t c=0; c<3; ++c) {
            l = std::max(l, box.xyz_max[c] - box.xyz_min[c]);
        }
        l /= double(nb);

        MeshGrob* sampling = MeshGrob::find_or_create(
            scene_graph(), sampling_name
        );
        sampling->clear();
        sampling->vertices.set_dimension(3);

        if(mesh_grob()->cells.nb() == 0) {
	    for(double x=box.xyz_min[0]+l/2.0; x<=box.xyz_max[0]; x += l) {
		for(double y=box.xyz_min[1]+l/2.0; y<=box.xyz_max[1]; y += l) {
		    double z = 0.0;
		    vec3 p(x,y,z);
		    sampling->vertices.create_vertex(p.data());
		}
	    }
        } else {
	    MeshCellsAABB AABB(*mesh_grob());
	    for(double x=box.xyz_min[0]+l/2.0; x<=box.xyz_max[0]; x += l) {
		for(double y=box.xyz_min[1]+l/2.0; y<=box.xyz_max[1]; y += l) {
		    for(double z=box.xyz_min[2]+l/2.0; z<=box.xyz_max[2]; z += l) {
			vec3 p(x,y,z);
			if(AABB.containing_tet(p) != index_t(-1)) {
			    sampling->vertices.create_vertex(p.data());
			}
		    }
		}
	    }
	}
        sampling->update();
    }

    void MeshGrobTransportCommands::perturb(
        double howmuch
    ) {
        for(index_t v=0; v<mesh_grob()->vertices.nb(); ++v) {
            Geom::mesh_vertex_ref(*mesh_grob(),v) += 2.0*howmuch *
                vec3(
                    Numeric::random_float64() - 0.5,
                    Numeric::random_float64() - 0.5,
                    Numeric::random_float64() - 0.5
                );
        }
        mesh_grob()->update();
    }

    void MeshGrobTransportCommands::init_Euler(
	const MeshGrobName& omega_name, EulerMode mode,
	const NewMeshGrobName& air_particles_name,
	const NewMeshGrobName& fluid_omega0_name

    ) {
	MeshGrob* omega = MeshGrob::find(scene_graph(), omega_name);
	if(omega == nullptr) {
	    Logger::err("OTM") << omega_name << ": no such mesh"
			       << std::endl;
	    return;
	}

	index_t dimension = (mode == EULER_2D ? 2 : 3);

	double air_fraction = 0.0;

	MeshGrob* air_particles_mesh = nullptr;
	index_t nb_air_particles = 0;
	const double* air_particles = nullptr;
	index_t air_particles_stride = 0;
	if(fluid_omega0_name != "") {
	    if(air_particles_name != "") {
		air_particles_mesh = MeshGrob::find(scene_graph(), air_particles_name);
		if(air_particles_mesh == nullptr) {
		    Logger::err("OTM") << air_particles_name << ": no such MeshGrob"
				       << std::endl;
		    return;
		}
		nb_air_particles = air_particles_mesh->vertices.nb();
		air_particles = air_particles_mesh->vertices.point_ptr(0);
		air_particles_stride = air_particles_mesh->vertices.dimension();
	    }

	    MeshGrob* fluid_omega0 = MeshGrob::find(scene_graph(), fluid_omega0_name);
	    if(fluid_omega0 == nullptr) {
		Logger::err("OTM") << fluid_omega0_name << ": no such MeshGrob"
				   << std::endl;
		return;
	    }

	    double omega_area = Geom::mesh_area(*omega, dimension);
	    double fluid_omega0_area = Geom::mesh_area(*fluid_omega0, dimension);
	    if(omega_area == 0.0) {
		Logger::err("OTM") << omega_name << ": area is zero !"
				   << std::endl;
		return;
	    }
	    if(fluid_omega0_area == 0.0) {
		Logger::err("OTM") << fluid_omega0_name << ": area is zero !"
				   << std::endl;
		return;
	    }
	    air_fraction = (omega_area - fluid_omega0_area) / omega_area;
	    Logger::out("OTM") << "Air fraction = " << air_fraction << std::endl;
	}


	vector<double> centroids(mesh_grob()->vertices.nb()*dimension);
	switch(mode) {
	    case EULER_2D: {
		if(omega->facets.nb() == 0) {
		    Logger::err("OTM") << omega_name << " has no facet"
				       << std::endl;
		    return;
		}
		vector<vec2> points_in(mesh_grob()->vertices.nb());
		FOR(v,mesh_grob()->vertices.nb()) {
		    points_in[v] = vec2(mesh_grob()->vertices.point_ptr(v));
		}
		compute_Laguerre_centroids_2d(
		    omega, points_in.size(),
		    points_in[0].data(), centroids.data(),
		    nullptr, false,
		    nb_air_particles, air_particles, air_particles_stride,
		    air_fraction
		);
	    } break;
	    case EULER_3D: {
		if(omega->cells.nb() == 0) {
		    Logger::err("OTM") << omega_name << " has no cell"
				       << std::endl;
		    return;
		}
		compute_Laguerre_centroids_3d(
		    omega, mesh_grob()->vertices.nb(),
		    mesh_grob()->vertices.point_ptr(0), centroids.data()
	        );
	    } break;
	    case EULER_ON_SURFACE: {
		if(omega->facets.nb() == 0) {
		    Logger::err("OTM") << omega_name << " has no facet"
				       << std::endl;
		    return;
		}
		compute_Laguerre_centroids_on_surface(
		    omega, mesh_grob()->vertices.nb(),
		    mesh_grob()->vertices.point_ptr(0), centroids.data()
		);
	    }
	}

	FOR(v,mesh_grob()->vertices.nb()) {
	    double* p = mesh_grob()->vertices.point_ptr(v);
	    FOR(c,dimension) {
		p[c] = centroids[dimension*v+c];
	    }
	}

	Attribute<double> mass(mesh_grob()->vertices.attributes(), "mass");
	bool all_zero = true;
	FOR(i, mesh_grob()->vertices.nb()) {
	    if(mass[i] != 0.0) {
		all_zero = false;
		break;
	    }
	}
	if(all_zero) {
	    Logger::warn("OTM") << "There was no mass, initializing to 1"
				<< std::endl;
	    FOR(i, mesh_grob()->vertices.nb()) {
		mass[i] = 1.0;
	    }
	}

	mesh_grob()->update();
    }


    //HERE

    void MeshGrobTransportCommands::get_density(const MeshGrobName& domain) {
        MeshGrob* M = MeshGrob::find(scene_graph(),domain);
        if(M == nullptr) {
            Logger::err("WarpDrive") << domain << ": no such MeshGrob"
                                     << std::endl;
            return;
        }

	double V = GEO::mesh_cells_volume(*M);

	Attribute<double> mass;
	mass.bind_if_is_defined(mesh_grob()->vertices.attributes(), "mass");

	if(!mass.is_bound()) {
	    Attribute<double> pt_density;
	    pt_density.bind_if_is_defined(
		mesh_grob()->vertices.attributes(), "density"
	    );
	    mass.bind(mesh_grob()->vertices.attributes(), "mass");
	    if(!pt_density.is_bound()) {
		for(index_t v:mesh_grob()->vertices) {
		    mass[v] = 1.0;
		}
	    } else {
		for(index_t v:mesh_grob()->vertices) {
		    mass[v] = 1.0 / pt_density[v];
			// V*pt_density[v] / double(mesh_grob()->vertices.nb());
		}
	    }
	}

	double Mtot = 0.0;
	for(index_t v:mesh_grob()->vertices) {
	    Mtot += mass[v];
	}
	Logger::out("Warpdrive") << "density = " << (Mtot/V) << std::endl;
	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::set_density(
	const MeshGrobName& domain, double density
    ) {
        MeshGrob* M = MeshGrob::find(scene_graph(),domain);
        if(M == nullptr) {
            Logger::err("WarpDrive") << domain << ": no such MeshGrob"
                                     << std::endl;
            return;
        }

	Attribute<double> mass(mesh_grob()->vertices.attributes(),"mass");
	double V = GEO::mesh_cells_volume(*M);
	// N*m/V = rho -> m = rho*V/N
	double m = density * V / double(mesh_grob()->vertices.nb());

	for(index_t v:mesh_grob()->vertices) {
	    mass[v] = m;
	}

	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::append_points(const MeshGrobName& points) {
        MeshGrob* M = MeshGrob::find(scene_graph(),points);
        if(M == nullptr) {
            Logger::err("WarpDrive") << points << ": no such MeshGrob"
                                     << std::endl;
            return;
        }

	Attribute<double> mass;
	Attribute<double> points_mass;
	mass.bind_if_is_defined(mesh_grob()->vertices.attributes(),"mass");
	points_mass.bind_if_is_defined(M->vertices.attributes(),"mass");

	if(!mass.is_bound() || !points_mass.is_bound()) {
	    Logger::err("WarpDrive") << "Missing mass attribute"
				     << std::endl;
	    return;
	}

	index_t v_ofs = mesh_grob()->vertices.nb();
	mesh_grob()->vertices.create_vertices(M->vertices.nb());

	for(index_t i = 0; i < M->vertices.nb()*3; ++i) {
	    mesh_grob()->vertices.point_ptr(v_ofs)[i] =
		M->vertices.point_ptr(0)[i];
	}

	for(index_t v: M->vertices) {
	    mass[v_ofs+v] = points_mass[v];
	}

	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::Euler2d(
        const MeshGrobName& omega_name,
        double tau,
        double epsilon,
        double g,
	index_t nb_iter,
	index_t save_every,
	index_t first_iter,
	bool show_RVD,
	bool show_centroids,
	bool verbose,
	index_t project_every,
	const NewMeshGrobName& air_particles_name,
	const NewMeshGrobName& fluid_omega0_name,
	bool physical,
	bool no_transport
    ) {
        MeshGrob* omega = MeshGrob::find(scene_graph(),omega_name);
        if(omega == nullptr) {
            Logger::err("Euler") << omega_name << ": no such MeshGrob"
                                 << std::endl;
            return;
        }

        if(omega->facets.nb() == 0) {
            Logger::err("Euler") << omega_name << ": does not have any facet"
                                 << std::endl;
            return ;
        }

	double air_fraction = 0.0;

	MeshGrob* air_particles_mesh = nullptr;
	index_t nb_air_particles = 0;
	const double* air_particles = nullptr;
	index_t air_particles_stride = 0;

	if(fluid_omega0_name != "") {

	    if(air_particles_name != "") {
		air_particles_mesh =
		    MeshGrob::find(scene_graph(), air_particles_name);
		if(air_particles_mesh == nullptr) {
		    Logger::err("OTM")
			<< air_particles_name << ": no such MeshGrob"
			<< std::endl;
		    return;
		} else {
		    nb_air_particles = air_particles_mesh->vertices.nb();
		    air_particles = air_particles_mesh->vertices.point_ptr(0);
		    air_particles_stride =
			air_particles_mesh->vertices.dimension();
		}
	    }

	    MeshGrob* fluid_omega0 =
		MeshGrob::find(scene_graph(), fluid_omega0_name);
	    if(fluid_omega0 == nullptr) {
		Logger::err("OTM") << fluid_omega0_name << ": no such MeshGrob"
				   << std::endl;
		return;
	    }

	    index_t dimension = 2;
	    double omega_area = Geom::mesh_area(*omega, dimension);
	    double fluid_omega0_area =
		Geom::mesh_area(*fluid_omega0, dimension);
	    if(omega_area == 0.0) {
		Logger::err("OTM") << omega_name << ": area is zero !"
				   << std::endl;
		return;
	    }
	    if(fluid_omega0_area == 0.0) {
		Logger::err("OTM") << fluid_omega0_name << ": area is zero !"
				   << std::endl;
		return;
	    }
	    air_fraction = (omega_area - fluid_omega0_area) / omega_area;
	    Logger::out("OTM")
		<< "Air fraction = " << air_fraction << std::endl;
	}


        index_t nb_pts = mesh_grob()->vertices.nb();

	vector<vec2> pos(nb_pts);
	FOR(v,nb_pts) {
	    pos[v] = vec2(mesh_grob()->vertices.point_ptr(v));
	}
        Attribute<double> m(mesh_grob()->vertices.attributes(),"mass");
        Attribute<vec2> V(mesh_grob()->vertices.attributes(),"V");
        vector<vec2> centroids(nb_pts);

        // CmdLine::set_arg("algo:predicates","exact");
        // CmdLine::set_arg("dbg:delaunay_benchmark",true);

	// Get bbox and center for initializing weights when some points are
	// off limits.
	double xyzmin[3];
	double xyzmax[3];
	get_bbox(*omega, xyzmin, xyzmax);
	double cx = 0.5*(xyzmin[0] + xyzmax[0]);
	double cy = 0.5*(xyzmin[1] + xyzmax[1]);
	double Rx = 0.5*(xyzmax[0] - xyzmin[0]);
	double Ry = 0.5*(xyzmax[1] - xyzmin[1]);

	vector<double> initial_weights(nb_pts);

	bool zero_iter = false;
	if(nb_iter == 0) {
	    nb_iter = 1;
	    zero_iter = true;
	}

        for(unsigned int k=1; k<=nb_iter; ++k) {
	    bool off_limits = false;

	    // Step 0: setup initial weights, to process off-limit points
	    double s = 1.0; // scaling
	    FOR(i, nb_pts) {
		vec2 p = pos[i];
		double cur_s = std::max(
		    ::fabs(p.x - cx) / Rx,
		    ::fabs(p.y - cy) / Ry
		);
		s = std::max(s, cur_s);
	    }
	    if(s > 1.0) {
		off_limits = true;
		std::cerr << "Off-limit ! setting weights accordingly"
			  << std::endl;
		s = 1.0 / (s * 1.01);
		vec2 c(cx,cy);
		FOR(i, nb_pts) {
		    vec2 p = pos[i];
		    initial_weights[i] =
			(s - 1.0) / 2.0 * length2(p) +
			(1.0 - s) * dot(c, p) ;
		}
	    } else {
		initial_weights.assign(nb_pts, 0.0);
	    }


            // Step 1: get Laguerre cells centroids

	    MeshGrob* RVD = nullptr;
	    bool RVD_existed = false;
	    if(show_RVD) {
		RVD_existed = (MeshGrob::find(scene_graph(),"RVD") != nullptr);
		RVD = MeshGrob::find_or_create(scene_graph(),"RVD");
	    }

	    bool centroids_existed = false;
	    if(show_centroids) {
		centroids_existed = (MeshGrob::find(scene_graph(),"centroids") != nullptr);
	    }

            compute_Laguerre_centroids_2d(
                omega, nb_pts,
		pos[0].data(),
                centroids[0].data(),
		RVD, verbose,
		nb_air_particles, air_particles, air_particles_stride,
		air_fraction,
		off_limits ? initial_weights.data() : nullptr,
		nullptr,
		no_transport ? 0 : 1000
            );

	    if(show_RVD) {
		Attribute<index_t> f_chart(RVD->facets.attributes(), "chart");
		Attribute<double> f_mass(RVD->facets.attributes(), "mass");
		FOR(f,RVD->facets.nb()) {
		    index_t chart = f_chart[f];
		    f_mass[f] =
			chart < mesh_grob()->vertices.nb() ? m[chart] : 0;
		}
		if(!RVD_existed) {
		    mesh_grob()->set_visible(false);
		    RVD->get_shader()->set_property("painting","ATTRIBUTE");
		    RVD->get_shader()->set_property("attribute","facets.mass");
		    RVD->get_shader()->set_property(
			"colormap","blue_red;true;0;false;false"
		    );
		    RVD->get_shader()->invoke_method("autorange");
		    RVD->set_visible(true);
		    omega->set_visible(false);
		}
		RVD->update();
	    }

	    if(show_centroids) {
		MeshGrob* mcentroids =
		    MeshGrob::find_or_create(scene_graph(), "centroids");
		Attribute<double> mass_centroid(
		    mcentroids->vertices.attributes(),"mass"
		);
		mcentroids->clear();
		mcentroids->vertices.set_dimension(3);
		mcentroids->vertices.create_vertices(centroids.size());
		FOR(i,centroids.size()) {
		    mcentroids->vertices.point_ptr(i)[0] = centroids[i].x;
		    mcentroids->vertices.point_ptr(i)[1] = centroids[i].y;
		    mass_centroid[i] = m[i];
		}
		if(!centroids_existed) {
		    mesh_grob()->set_visible(false);
		    mcentroids->get_shader()->set_property("vertices_style","true; 0 1 0 1; 2");
		    mcentroids->get_shader()->set_property("painting","ATTRIBUTE");
		    mcentroids->get_shader()->set_property("attribute","vertices.mass");
		    mcentroids->get_shader()->set_property(
			"colormap","blue_red;true;0;false;false"
		    );
		    mcentroids->get_shader()->invoke_method("autorange");
		    mcentroids->set_visible(true);
		}
		mcentroids->update();
	    }

	    if(zero_iter) {
		if(RVD != nullptr) {
		    RVD->update();
		    if(!verbose) {
			RVD->redraw();
		    }
		}
		break;
	    }

            // Step 2: update speeds

            double inveps2 = 1.0 / geo_sqr(epsilon);

	    if(physical) {
		for(index_t v=0; v<nb_pts; ++v) {
		    V[v] += tau*(
			1.0 / m[v] * inveps2 * (
			    centroids[v] - vec2(mesh_grob()->vertices.point_ptr(v))
			) +
			vec2(0.0, -g)
		    );
		}
	    } else {
		for(index_t v=0; v<nb_pts; ++v) {
		    V[v] += tau*(
			inveps2*(
			    centroids[v] - vec2(mesh_grob()->vertices.point_ptr(v))
			) +
			m[v] * vec2(0.0, -g)
		    );
		}
	    }

            // Step 3: update positions

	    if(project_every != 0 && ((k % project_every) == 0)) {
		for(index_t v=0; v<nb_pts; ++v) {
		    double* p = mesh_grob()->vertices.point_ptr(v);
		    p[0] = centroids[v].x;
		    p[1] = centroids[v].y;
		    pos[v].x = centroids[v].x;
		    pos[v].y = centroids[v].y;
		}
	    }

            for(index_t v=0; v<nb_pts; ++v) {
		double* p = mesh_grob()->vertices.point_ptr(v);
		p[0] += tau*V[v].x;
		p[1] += tau*V[v].y;


		// Brute force for now...
		{
		    if(p[0] < xyzmin[0]) {
			p[0] = xyzmin[0];
			V[v].x /= 2.0;
		    } else if(p[0] > xyzmax[0]) {
			p[0] = xyzmax[0];
			V[v].x /= 2.0;
		    }

		    if(p[1] < xyzmin[1]) {
			p[1] = xyzmin[1];
			V[v].y /= 2.0;
		    } else if(p[1] > xyzmax[1]) {
			p[1] = xyzmax[1];
			V[v].y /= 2.0;
		    }
		}

		pos[v].x = p[0];
		pos[v].y = p[1];
            }


	    if(save_every != 0 && ((k % save_every) == 0)) {
		if(verbose) {
		    Logger::out("Euler") << "Saving timestep..." << std::endl;
		}

		std::string iter_str = String::to_string(k + first_iter);
		while(iter_str.length() < 5) {
		    iter_str = "0" + iter_str;
		}

		scene_graph()->save_current_object(
		    "Euler_timestep_" + iter_str + ".graphite"
		);
	    }


            omega->update();
            mesh_grob()->update();
	    // Need to trigger a graphics update (not needed in verbose
	    // mode since message displays trigger graphics updates).
	    if(!verbose) {
		mesh_grob()->redraw();
	    }
        }
    }

    void MeshGrobTransportCommands::shell_mesh(
        const MeshGrobName& shell_name,
        double inner_density,
        double outer_density_max,
        double outer_density_min,
        double gamma,
	bool shell_only
    ) {
        MeshGrob* shell = MeshGrob::find(scene_graph(), shell_name);
        if(shell == nullptr) {
            Logger::err("shell") << shell_name << ": no such MeshGrob"
                                 << std::endl;
            return;
        }

	if(shell_only) {

	    // Add outer shell and tetrahedralize
	    {
		append_surface_mesh(mesh_grob(), shell);
		mesh_grob()->vertices.remove_isolated();
		mesh_grob()->facets.connect();
		mesh_tetrahedralize(*mesh_grob(), false, true, 1.0, true);
	    }

	    // Remove the tets that are inside the inner shell
	    {
		Attribute<index_t> region(mesh_grob()->cells.attributes(),"region");
		vector<index_t> remove_tet(mesh_grob()->cells.nb());
		index_t shell_id = index_t(-1);
		for(index_t t : mesh_grob()->cells) {
		    for(index_t le=0; le<4; ++le) {
			if(mesh_grob()->cells.adjacent(t,le) == index_t(-1)) {
			    shell_id = region[t];
			    break;
			}
		    }
		}
		for(index_t t : mesh_grob()->cells) {
		    remove_tet[t] = (region[t] != shell_id);
		}
		mesh_grob()->cells.delete_elements(remove_tet);
	    }

	    mesh_grob()->update();
	    return;
	}


        if(mesh_grob()->cells.nb() == 0) {
            Logger::out("Shell") << "Tetrahedralizing mesh" << std::endl;
            mesh_tetrahedralize(*mesh_grob(), false, true, 2.0);
            mesh_grob()->update();
        }

        Mesh initial_surface;
        initial_surface.vertices.set_dimension(3);
        append_surface_mesh(&initial_surface, mesh_grob());
        MeshFacetsAABB AABB(initial_surface);

        Mesh tets;
        tets.vertices.set_dimension(3);
        append_surface_mesh(&tets, mesh_grob());
        append_surface_mesh(&tets, shell);
        tets.vertices.remove_isolated();
        tets.facets.connect();
        mesh_tetrahedralize(tets, false, true, 1.0, shell_only);

        index_t v_offset = mesh_grob()->vertices.nb();

        append_volume_mesh(mesh_grob(), &tets);
        mesh_grob()->cells.connect();
        mesh_grob()->cells.compute_borders();

        double dmin = 1e30;
        double dmax = -1e30;

        Attribute<double> weight(mesh_grob()->vertices.attributes(), "weight");

        for(index_t v=0; v<v_offset; ++v) {
            weight[v] = inner_density;
        }

        for(index_t v=v_offset; v<mesh_grob()->vertices.nb(); ++v) {
            double d = ::sqrt(
                AABB.squared_distance(vec3(mesh_grob()->vertices.point_ptr(v)))
            );
            dmin = std::min(dmin,d);
            dmax = std::max(dmax,d);
        }

        for(index_t v=v_offset; v<mesh_grob()->vertices.nb(); ++v) {
            double d = ::sqrt(
                AABB.squared_distance(vec3(mesh_grob()->vertices.point_ptr(v)))
            );
            d = (dmax-d) / (dmax-dmin);
            d = pow(d,gamma);
            weight[v] = d*outer_density_max + (1.0-d)*outer_density_min;
        }

        mesh_grob()->update();
    }

    void MeshGrobTransportCommands::symmetric_transport_3d(
        const MeshGrobName& other_name,
        index_t nb_points,
        const NewMeshGrobName& thissampling_name,
        const NewMeshGrobName& othersampling_name,
        index_t nb_iter
    ) {
        MeshGrob* other = MeshGrob::find(scene_graph(), other_name);
        if(other == nullptr) {
            Logger::err("Transport") << other_name << ": no such meshgrob"
                                     << std::endl;
        }

        if(mesh_grob()->cells.nb() == 0) {
            Logger::err("Transport") << "current mesh does not have tets"
                                     << std::endl;
            return ;
        }

        if(other->cells.nb() == 0) {
            Logger::err("Transport") << other_name << " does not have tets"
                                     << std::endl;
            return ;
        }

        MeshGrob* thissampling = MeshGrob::find_or_create(
            scene_graph(), thissampling_name
        );
        thissampling->clear();
        thissampling->vertices.set_dimension(3);
        thissampling->vertices.create_vertices(nb_points);

        MeshGrob* othersampling = MeshGrob::find_or_create(
            scene_graph(), othersampling_name
        );

        othersampling->clear();
        othersampling->vertices.set_dimension(3);
        othersampling->vertices.create_vertices(nb_points);

        /*
        {
            Attribute<double> thispotential(
                thissampling->vertices.attributes(), "psi"
            );
            Attribute<double> otherpotential(
                othersampling->vertices.attributes(), "psi"
            );
        }
        */


        // Step 1: create other sampling

        vector<index_t> levels;
        double ratio = 0.125;
        bool BRIO = true;
        bool multilevel = true;
        double epsilon = 0.01;
        double regularization = 0.0;
        index_t nb_OTM_iter = 1000;

        {
            CentroidalVoronoiTesselation CVT(other, 0, "NN");
	    CVT.set_volumetric(true);
            sample(
                CVT, nb_points, false /* do not project on border */, BRIO,
                multilevel, ratio, &levels
            );
            othersampling->vertices.assign_points(
                CVT.embedding(0), CVT.dimension(), CVT.nb_points()
            );
            othersampling->update();
        }


        mesh_grob()->vertices.set_dimension(4);
        other->vertices.set_dimension(4);

        for(index_t k=0; k<nb_iter; ++k) {
            {
                Stopwatch W("OTM 1->2");
                OptimalTransportMap3d OTM(mesh_grob(), "default", BRIO);
                OTM.set_points(
                    othersampling->vertices.nb(),
		    othersampling->vertices.point_ptr(0)
                );
                OTM.set_epsilon(epsilon);
                OTM.set_regularization(regularization);
                OTM.optimize_levels(levels, nb_OTM_iter);
                OTM.compute_Laguerre_centroids(
		    thissampling->vertices.point_ptr(0)
		);

                if(k == nb_iter-1) {
                    MeshGrob* RVD_mesh =
                        MeshGrob::find_or_create(scene_graph(), "RVD1to2");
                    OTM.get_RVD(*RVD_mesh);
                    RVD_mesh->update();
                }
            }


            thissampling->update();

            {
                Stopwatch W("OTM 2->1");
                OptimalTransportMap3d OTM(other, "default", BRIO);
                OTM.set_points(
                    thissampling->vertices.nb(),
		    thissampling->vertices.point_ptr(0)
                );
                OTM.set_epsilon(epsilon);
                OTM.set_regularization(regularization);

                OTM.optimize_levels(levels, nb_OTM_iter);
                OTM.compute_Laguerre_centroids(
		    othersampling->vertices.point_ptr(0)
		);

                if(k == nb_iter-1) {
                    mesh_grob()->vertices.set_dimension(3);
                    CentroidalVoronoiTesselation CVT(mesh_grob(), 0, "NN");
                    CVT.set_points(
			thissampling->vertices.nb(),
			thissampling->vertices.point_ptr(0)
		    );
                    CVT.delaunay()->set_vertices(
			thissampling->vertices.nb(),
			thissampling->vertices.point_ptr(0)
		    );
                    CVT.set_volumetric(true);

                    MeshGrob* morph = MeshGrob::find_or_create(
                        scene_graph(), "morph"
                    );
                    morph->clear();
                    morph->vertices.set_dimension(6);

                    Logger::div("Morphing");
                    Logger::out("OTM") <<  "Time-coherent triangulation."
				       << std::endl;

                    compute_morph(CVT, OTM, *morph, true);
                    morph->update();
                    morph->get_shader()->set_property("animate","true");

                    {
                        MeshGrob* RVD_mesh =
                            MeshGrob::find_or_create(scene_graph(), "RVD2to1");
                        OTM.get_RVD(*RVD_mesh);
                        RVD_mesh->update();
                    }
                }
            }
            othersampling->update();

        }

        mesh_grob()->vertices.set_dimension(3);
        mesh_grob()->update();
        other->vertices.set_dimension(3);
        other->update();

    }

    void MeshGrobTransportCommands::isoarea_Laguerre_2d(
	const NewMeshGrobName& points_name,
	const NewMeshGrobName& laguerre_name,
	const NewMeshGrobName& air_name,
	double air_fraction,
	bool surface3D,
	index_t nb_points,
	index_t nb_iter,
	double epsilon,
	double regul,
	OTLinearSolver solver
    ) {
	MeshGrob* points = MeshGrob::find_or_create(
	    scene_graph(), points_name
	);

	MeshGrob* laguerre = MeshGrob::find_or_create(
	    scene_graph(), laguerre_name
	);
	laguerre->clear();

	// Compute initial sampling.
	if(nb_points != 0) {
	    points->clear();
	    points->vertices.set_dimension(3);
	    points->vertices.create_vertices(nb_points);
	    Attribute<double> dummy;
	    mesh_generate_random_samples_on_surface<3>(
		*mesh_grob(),
		points->vertices.point_ptr(0),
		nb_points,
		dummy
	    );
	} else {
	    nb_points = points->vertices.nb();
	    if(nb_points == 0) {
		Logger::err("OTM") << "No points" << std::endl;
		return;
	    }
	}

	Attribute<double> nu;
	nu.bind_if_is_defined(points->vertices.attributes(),"nu");

	MeshGrob* air = nullptr;
	if(air_name != "") {
	    air = MeshGrob::find(scene_graph(), air_name);
	    if(air == nullptr) {
		Logger::err("OTM") << air_name << ": no such MeshGrob"
				   << std::endl;
		return;
	    }
	}

	// Compute optimal transport map.
	if(surface3D) {
	    // Everything happens in dimension 4 (power diagram is seen
	    // as Voronoi diagram in dimension 4), therefore the dimension
	    // of M1 needs to be changed as well (even if it is not used).
	    mesh_grob()->vertices.set_dimension(4);
	    mesh_grob()->update();

	    OptimalTransportMapOnSurface OTM(mesh_grob());
	    if(air != nullptr) {
		OTM.set_air_particles(
		    air->vertices.nb(),
		    air->vertices.point_ptr(0),
		    air->vertices.dimension(),
		    air_fraction
		);
	    }
	    OTM.set_points(
		points->vertices.nb(),
		points->vertices.point_ptr(0),
		points->vertices.dimension()
	    );
	    if(nu.is_bound()) {
		FOR(i,points->vertices.nb()) {
		    OTM.set_nu(i,nu[i]);
		}
	    }
	    OTM.set_regularization(1e-3);
	    OTM.set_epsilon(epsilon);
	    OTM.set_Newton(true);
	    OTM.optimize(nb_iter);
	    OTM.get_RVD(*laguerre);

	    mesh_grob()->vertices.set_dimension(3);
	    mesh_grob()->update();
	} else {
	    OptimalTransportMap2d OTM(mesh_grob());
	    if(air != nullptr) {
		OTM.set_air_particles(
		    air->vertices.nb(),
		    air->vertices.point_ptr(0),
		    air->vertices.dimension(),
		    air_fraction
		);
	    }
	    OTM.set_points(
		points->vertices.nb(),
		points->vertices.point_ptr(0),
		points->vertices.dimension()
	    );
	    if(nu.is_bound()) {
		FOR(i,points->vertices.nb()) {
		    OTM.set_nu(i,nu[i]);
		}
	    }
	    OTM.set_regularization(regul);
	    OTM.set_linear_solver(solver);
	    OTM.set_epsilon(epsilon);
	    OTM.set_Newton(true);
	    OTM.optimize(nb_iter);
	    OTM.get_RVD(*laguerre);
	}

	points->update();
	laguerre->update();
    }


    void MeshGrobTransportCommands::crop_domain(
	const MeshGrobName& domain_name
    ) {
	MeshGrob* domain = MeshGrob::find(scene_graph(), domain_name);
	MeshGrob* points = mesh_grob();

	if(domain == nullptr) {
	    Logger::err("OTM") << "Did not find domain" << std::endl;
	    return;
	}

	if(domain->cells.nb() == 0) {
	    Logger::err("OTM") << "Domain has no cell" << std::endl;
	    return;
	}


	domain->lock_graphics();
	MeshCellsAABB AABB(*domain);
	vector<index_t> to_delete(points->vertices.nb(),0);
	FOR(v,points->vertices.nb()) {
	    if(
		AABB.containing_tet(vec3(points->vertices.point_ptr(v))) ==
				    MeshCellsAABB::NO_TET
	    ) {
		to_delete[v] = 1;
	    }
	}
	points->vertices.delete_elements(to_delete);
	domain->unlock_graphics();
	domain->update();
	points->update();
    }

    void MeshGrobTransportCommands::crop_region(
	double xmin,
	double ymin,
	double zmin,
	double xmax,
	double ymax,
	double zmax,
	bool cell_center,
	bool per_vertices
    ) {
	if(per_vertices) {
	    vector<index_t> delete_vertex(mesh_grob()->vertices.nb(),0);
	    FOR(v, mesh_grob()->vertices.nb()) {
		const double* p = mesh_grob()->vertices.point_ptr(v);
		if(
		    p[0] < xmin || p[0] > xmax ||
		    p[1] < ymin || p[1] > ymax ||
		    p[2] < zmin || p[2] > zmax
		) {
		    delete_vertex[v] = 1;
		}
	    }
	    mesh_grob()->vertices.delete_elements(delete_vertex);
	    mesh_grob()->update();
	    return;
	}

	if(mesh_grob()->cells.nb() == 0) {
	    Logger::err("crop") << "Mesh has no cell"
				<< std::endl;
	    return;
	}
	vector<index_t> delete_cell(mesh_grob()->cells.nb(),0);
	for(index_t c=0; c<mesh_grob()->cells.nb(); ++c) {
	    double cxmin=Numeric::max_float64();
	    double cymin=Numeric::max_float64();
	    double czmin=Numeric::max_float64();
	    double cxmax=Numeric::min_float64();
	    double cymax=Numeric::min_float64();
	    double czmax=Numeric::min_float64();
	    vec3 center(0.0, 0.0, 0.0);
	    for(index_t lv=0; lv<mesh_grob()->cells.nb_vertices(c); ++lv) {
		index_t v = mesh_grob()->cells.vertex(c,lv);
		const double* p = mesh_grob()->vertices.point_ptr(v);
		cxmin = std::min(cxmin, p[0]);
		cxmax = std::max(cxmax, p[0]);
		cymin = std::min(cxmin, p[1]);
		cymax = std::max(cxmax, p[1]);
		czmin = std::min(cxmin, p[2]);
		czmax = std::max(cxmax, p[2]);
		center += vec3(p);
	    }
	    center = 1.0 / double(mesh_grob()->cells.nb_vertices(c)) * center;

	    if(cell_center) {
		if(center.x < xmin || center.y < ymin || center.z < zmin ||
		   center.x > xmax || center.y > ymax || center.z > zmax) {
		    delete_cell[c] = 1;
		}
	    } else {
		if(cxmin < xmin || cymin < ymin || czmin < zmin ||
		   cxmax > xmax || cymax > ymax || czmax > zmax) {
		    delete_cell[c] = 1;
		}
	    }
	}
	mesh_grob()->cells.delete_elements(delete_cell);
	mesh_grob()->cells.compute_borders();
	// Needed, because some vertices were still attached to
	// the old surfacic border before it was recomputed.
	mesh_grob()->vertices.remove_isolated();
	mesh_grob()->update();
    }


    void MeshGrobTransportCommands::optimize_VSDM(
	double affinity,
	index_t nb_iter,
	const MeshGrobName& grid_name,
	index_t nb_subd,
	const NewMeshGrobName& subd_name
    ) {
	MeshGrob* grid = MeshGrob::find(scene_graph(), grid_name);

	if(grid == nullptr) {
	    Logger::err("VSDM")
		<< grid_name << ": no such MeshGrob"
		<< std::endl;
	    return;
	}

	if(grid == mesh_grob()) {
	    Logger::err("VSDM")
		<< "Source and target cannot be the same surface"
		<< std::endl;
	    return;
	}

	MeshGrob* subd = nullptr;
	if(nb_subd != 0) {
	    subd = MeshGrob::find_or_create(scene_graph(), subd_name);
	    subd->clear();
	}

	try {
	    VisualVSDM vsdm(grid, mesh_grob());
	    ProgressTask progress("VSDM",nb_iter);
	    vsdm.set_progress(&progress);
	    vsdm.set_affinity(affinity);

	    if(nb_subd != 0) {
		vsdm.set_subdivision_surface(subd, nb_subd);
	    }

	    vsdm.optimize(nb_iter);
	} catch(...) {
	}

	grid->update();
    }


    void MeshGrobTransportCommands::copy_t0(
	const std::string& t0_mesh_name
    ) {
	MeshGrob* t0_mesh = MeshGrob::find(scene_graph(), t0_mesh_name);
	if(t0_mesh == nullptr) {
	    Logger::err("copy_t0") << t0_mesh_name << ": no such MeshGrob"
				   << std::endl;
	    return;
	}
	if(t0_mesh->vertices.nb() != mesh_grob()->vertices.nb()) {
	    Logger::err("copy_t0")
		<< t0_mesh_name << ": does not have the same number of vertices as this mesh"
		<< std::endl;
	    return;
	}
	Attribute<double> t0;
	t0.bind_if_is_defined(mesh_grob()->vertices.attributes(), "t0");
	if(!t0.is_bound()) {
	    t0.create_vector_attribute(mesh_grob()->vertices.attributes(), "t0", 3);
	}
	if(!t0.is_bound() || t0.dimension() != 3) {
	    Logger::err("copy_t0") << "t0 attribute already exists with wrong type or dimension"
				   << std::endl;
	    return;
	}
	FOR(v,mesh_grob()->vertices.nb()) {
	    const double* p_t0 = t0_mesh->vertices.point_ptr(v);
	    t0[3*v  ] = p_t0[0];
	    t0[3*v+1] = p_t0[1];
	    t0[3*v+2] = p_t0[2];
	}
	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::compute_velocity(
	const std::string& velocity_name,
	const std::string& speed_vector_name
    ) {
	Attribute<vec3> speed_vector;
	speed_vector.bind_if_is_defined(
	    mesh_grob()->vertices.attributes(), speed_vector_name
	);
	if(!speed_vector.is_bound()) {
	    Logger::err("OTM") << speed_vector_name
			       << ": no such vector attribute" << std::endl;
	    return;
	}
	Attribute<double> velocity(
	    mesh_grob()->vertices.attributes(), velocity_name
	);
	FOR(v,mesh_grob()->vertices.nb()) {
	    velocity[v] = length(speed_vector[v]);
	}
	mesh_grob()->update();
    }

    /**********************************************************************/

    /**
     * \brief Computes the inverfaces between the power cells of a
     *  Merigot-Gallouet fluid.
     */
    class DualInterfacePolyhedronCallback : public RVDPolyhedronCallback {
    public:
	/**
	 * \brief DualInterfacePolyhedronCallback constructor.
	 */
	DualInterfacePolyhedronCallback(
	    const Attribute<double>& mass, Mesh* interface
	) :
	    mass_(mass),
	    interface_(interface),
	    cur_v_(0)
	{
	    interface->vertices.set_dimension(3);
	}

	/**
	 * \copydoc RVDPolyhedronCallback::operator()
	 */
	void operator() (
	    index_t v,
	    index_t t,
	    const GEOGen::ConvexCell& C
	) const override {

	    geo_argused(t);

	    for(index_t cv = 0; cv < C.max_v(); ++cv) {
		signed_index_t ct = C.vertex_triangle(cv);
		if(ct == -1) {
		    continue;
		}
		geo_debug_assert(C.triangle_is_used(index_t(ct)));

		index_t v_adj = index_t(-1);
		signed_index_t adjacent = C.vertex_id(cv);
		if(adjacent > 0) {
		    // Positive adjacent indices correspond to
		    // Voronoi seed - Voronoi seed link
		    v_adj = index_t(adjacent - 1);
		}
		bool on_border = (adjacent == 0);
		// Zero adjacent indices corresponds to
		// tet facet on border.

		//   In non-weighted mode, we only traverse the facets on
		// the boundary of the Voronoi cells.
		if((v_adj == index_t(-1) && !on_border) || (v == v_adj)) {
		    continue;
		}

		if(on_border && mass_[v] < 2.0) {
		    continue;
		}

		if(!on_border && mass_[v] <= mass_[v_adj]) {
		    continue;
		}


		GEOGen::ConvexCell::Corner first(
		    index_t(ct), C.find_triangle_vertex(index_t(ct), cv)
		);
		GEOGen::ConvexCell::Corner c = first;
		do {
		    const GEOGen::Vertex* V = &C.triangle_dual(c.t);
		    interface_->vertices.create_vertex(V->point());
		    C.move_to_next_around_vertex(c);
		} while(c != first);

		index_t nb_new_v = interface_->vertices.nb() - cur_v_;
		index_t f = interface_->facets.create_polygon(
		    nb_new_v
		);
		FOR(i,nb_new_v) {
		    interface_->facets.set_vertex(f, i, cur_v_+i);
		}
		cur_v_ = interface_->vertices.nb();
	    }
	}

    private:
	const Attribute<double>& mass_;
	Mesh* interface_;
	mutable index_t cur_v_;
    };


    /**
     * \brief Computes the inverfaces between the power cells of a
     *  Merigot-Gallouet fluid in primal form.
     */
    class PrimalInterfacePolyhedronCallback : public RVDPolyhedronCallback {
    public:
	/**
	 * \brief PrimalInterfacePolyhedronCallback constructor.
	 */
	PrimalInterfacePolyhedronCallback(
	    const Attribute<double>& mass, Mesh* interface, Delaunay* delaunay
	) :
	    mass_(mass),
	    interface_(interface),
	    delaunay_(delaunay)
	{
	    interface->vertices.set_dimension(3);
	}

	/**
	 * \copydoc RVDPolyhedronCallback::operator()
	 */
	void operator() (
	    index_t v,
	    index_t t,
	    const GEOGen::ConvexCell& C
	) const override {
	    geo_argused(t);

	    for(index_t ct = 0; ct < C.max_t(); ++ct) {
		if(!C.triangle_is_used(ct)) {
		    continue;
		}

		signed_index_t sv1 = C.vertex_id(C.triangle_vertex(ct,0)) - 1;
		signed_index_t sv2 = C.vertex_id(C.triangle_vertex(ct,1)) - 1;
		signed_index_t sv3 = C.vertex_id(C.triangle_vertex(ct,2)) - 1;

		if(sv1 < 0 || sv2 < 0 || sv3 < 0) {
		    continue;
		}

		index_t v1 = index_t(sv1);
		index_t v2 = index_t(sv2);
		index_t v3 = index_t(sv3);

		if(v1 < v || v2 < v || v3 < v) {
		    continue;
		}

		{
		    // "Marching tet" tables
		    // ---------------------

		    // tet_edge[e][v] gives for each edge
		    // e of a tet the local indices of its two
		    // vertices.
		    static index_t tet_edges[6][2] = {
			{0,1},{1,2},{2,0},
			{3,0},{3,1},{3,2}
		    };

		    // tet_cases[config] gives the list of vertices
		    // of the intersection polygon between the tet.
		    // returned vertices indices correspond to local
		    // edge indices (vertices are intersections on the
		    // edges).
		    static const index_t X = index_t(-1);
		    static index_t tet_cases[16][4] = {
			{X,X,X,X}, // 0000
			{0,2,3,X}, // 0001
			{0,1,4,X}, // 0010
			{1,4,2,3}, // 0011
			{1,2,5,X}, // 0100
			{0,1,3,5}, // 0101
			{0,2,4,5}, // 0110
			{3,4,5,X}, // 0111
			{3,4,5,X}, // 1000
			{0,2,4,5}, // 1001
			{0,1,3,5}, // 1010
			{1,2,5,X}, // 1011
			{1,4,2,3}, // 1100
			{0,1,4,X}, // 1101
			{0,2,3,X}, // 1110
			{X,X,X,X}  // 1111
		    };


		    //   The four global indices of the tetrahedron
		    // vertices.
		    index_t old_v[4];
		    old_v[0] = v;
		    old_v[1] = v1;
		    old_v[2] = v2;
		    old_v[3] = v3;

		    //   The global indices of the newly created
		    // vertices.
		    index_t new_idx[6];

		    FOR(e,6) {
			index_t vv1 = old_v[tet_edges[e][0]];
			index_t vv2 = old_v[tet_edges[e][1]];
			const double* p1 = delaunay_->vertex_ptr(vv1);
			const double* p2 = delaunay_->vertex_ptr(vv2);
			if(mass_[vv1] != mass_[vv2]) {
			    index_t new_v = interface_->vertices.create_vertex();
			    interface_->vertices.point_ptr(new_v)[0] = 0.5 * (p1[0] + p2[0]);
			    interface_->vertices.point_ptr(new_v)[1] = 0.5 * (p1[1] + p2[1]);
			    interface_->vertices.point_ptr(new_v)[2] = 0.5 * (p1[2] + p2[2]);
			    new_idx[e] = new_v;
			}
		    }
		    index_t code = 0;
		    FOR(lv,4) {
			index_t ov = old_v[lv];
			code = code | (index_t(mass_[ov] > 2.0) << lv);
		    }
		    if(tet_cases[code][0] != X) {
			if(tet_cases[code][3] == X) {
			    interface_->facets.create_triangle(
				new_idx[tet_cases[code][0]],
				new_idx[tet_cases[code][1]],
				new_idx[tet_cases[code][2]]
			     );
			} else {
			    interface_->facets.create_quad(
				new_idx[tet_cases[code][0]],
				new_idx[tet_cases[code][1]],
				new_idx[tet_cases[code][3]],
				new_idx[tet_cases[code][2]]
			    );
			}
		    }
		}
	    }
	}

    private:
	const Attribute<double>& mass_;
	Mesh* interface_;
	Delaunay* delaunay_;
    };

    /**********************************************************************/


    void MeshGrobTransportCommands::compute_interface(
	const MeshGrobName& domain_name,
	const NewMeshGrobName& interface_name,
	const NewMeshGrobName& clip_region_name,
	bool primal
    ) {
	MeshGrob* domain = MeshGrob::find(scene_graph(), domain_name);
	MeshGrob* points = mesh_grob();

	if(domain == nullptr) {
	    Logger::err("OTM") << "Did not find domain" << std::endl;
	    return;
	}

	Attribute<double> mass;
	mass.bind_if_is_defined(points->vertices.attributes(), "mass");
	if(!mass.is_bound()) {
	    Logger::err("OTM") <<"missing mass attribute"
			       << std::endl;
	    return;
	}

	if(domain->cells.nb() == 0) {
	    Logger::err("OTM") << "Domain has no cell" << std::endl;
	    return;
	}

	MeshGrob* clip = nullptr;
	if(clip_region_name != "") {
	    clip = MeshGrob::find(scene_graph(), clip_region_name);
	    if(clip == nullptr) {
		Logger::err("OTM") << clip_region_name << ": no such mesh"
				   << std::endl;
		return;
	    }
	    if(clip->cells.nb() == 0) {
		Logger::err("OTM") << clip_region_name << " has no cell"
				   << std::endl;
		return;
	    }
	    clip->vertices.set_dimension(4);
	}


	MeshGrob* interface = MeshGrob::find_or_create(
	    scene_graph(), interface_name
	);
	interface->clear();
	interface->vertices.set_dimension(3);


	domain->vertices.set_dimension(4);

	OptimalTransportMap3d OTM(domain, "BPOW", false);
	OTM.set_regularization(1e-3);
	OTM.set_Newton(true);
	OTM.set_points(points->vertices.nb(), points->vertices.point_ptr(0));
	OTM.set_epsilon(0.01);
	OTM.optimize(1000);

	RestrictedVoronoiDiagram_var RVD = OTM.RVD();
	if(clip != nullptr) {
	    RVD = RestrictedVoronoiDiagram::create(RVD->delaunay(), clip);
	}

	if(primal) {
	    PrimalInterfacePolyhedronCallback iface_cb(
		mass, interface, RVD->delaunay()
	    );
	    RVD->for_each_polyhedron(iface_cb, false, false, false);
	    interface->vertices.remove_isolated();
	} else {
	    DualInterfacePolyhedronCallback iface_cb(mass, interface);
	    RVD->for_each_polyhedron(iface_cb, false, false, false);
	}

	domain->vertices.set_dimension(3);

	mesh_repair(
	    *interface,
	    GEO::MeshRepairMode(
		GEO::MESH_REPAIR_COLOCATE | GEO::MESH_REPAIR_DUP_F
	    ),
	    1e-6 * (0.01 * bbox_diagonal(*interface))
	);

	if(clip != nullptr) {
	    clip->vertices.set_dimension(3);
	    clip->update();
	}

	interface->update();
    }

    void MeshGrobTransportCommands::Euler3d(
        const MeshGrobName& omega_name,
        double tau,
        double epsilon,
        double g,
	index_t nb_iter,
	index_t save_every,
	index_t first_iter,
	bool compute_interface,
	bool split_interface,
	bool verbose,
	index_t project_every,
	bool physical
    ) {
        MeshGrob* omega = MeshGrob::find(scene_graph(),omega_name);
        if(omega == nullptr) {
            Logger::err("Euler") << omega_name << ": no such MeshGrob"
                                 << std::endl;
            return;
        }

        if(omega->cells.nb() == 0) {
            Logger::err("Euler") << omega_name << ": does not have any cell"
                                 << std::endl;
            return ;
        }

        index_t nb_pts = mesh_grob()->vertices.nb();

	bool zero_iter = false;
	if(nb_iter == 0) {
	    nb_iter = 1;
	    zero_iter = true;
	}
	// HERE

        Attribute<double> m(mesh_grob()->vertices.attributes(),"mass");
        Attribute<vec3> V(mesh_grob()->vertices.attributes(),"V");
        vector<vec3> centroids(nb_pts);

        // CmdLine::set_arg("algo:predicates","exact");
        // CmdLine::set_arg("dbg:delaunay_benchmark",true);

	MeshGrob* interface = nullptr;
	if(compute_interface) {
	    interface = MeshGrob::find_or_create(scene_graph(), "interface");
	}

        for(unsigned int k=1; k<=nb_iter; ++k) {
	    Stopwatch W("Timestep");

	    if(verbose) {
		Logger::out("Euler")
		    << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> "
		    << "Time step = " << k << std::endl;
	    }

            // Step 1: get Laguerre cells centroids

	    if(compute_interface) {
		Mesh tmp_interface;
		tmp_interface.vertices.set_dimension(3);
		DualInterfacePolyhedronCallback iface_cb(m, &tmp_interface);
		compute_Laguerre_centroids_3d(
		    omega, nb_pts,
		    mesh_grob()->vertices.point_ptr(0),
		    centroids[0].data(),
		    &iface_cb,
		    verbose,
		    zero_iter ? 0 : 2000
		);
		mesh_repair(
		    tmp_interface,
		    GEO::MeshRepairMode(
			GEO::MESH_REPAIR_COLOCATE | GEO::MESH_REPAIR_DUP_F
		    ),
		    1e-6 * (0.01 * bbox_diagonal(tmp_interface))
		);
		interface->copy(tmp_interface);
		if(split_interface) {
		    mesh_split_catmull_clark(*interface);
		    mesh_split_catmull_clark(*interface);
		}
		interface->update();
	    } else {
		compute_Laguerre_centroids_3d(
		    omega, nb_pts,
		    mesh_grob()->vertices.point_ptr(0),
		    centroids[0].data(),
		    nullptr,
		    verbose
		);
	    }

	    if(zero_iter) {
		if(interface != nullptr) {
		    interface->update();
		    if(!verbose) {
			interface->redraw();
		    }
		}
		break;
	    }

            // Step 2: update speeds

            double inveps2 = 1.0 / geo_sqr(epsilon);
	    if(physical) {
		for(index_t v=0; v<nb_pts; ++v) {
		    V[v] += tau*(
			(inveps2 / m[v]) * (centroids[v] - Geom::mesh_vertex(*mesh_grob(),v)) +
			vec3(0.0, 0.0, -g)
		    );
		}
	    } else {
		for(index_t v=0; v<nb_pts; ++v) {
		    V[v] += tau*(
			inveps2*(centroids[v] - Geom::mesh_vertex(*mesh_grob(),v)) +
			m[v] * vec3(0.0, 0.0, -g)
		    );
		}
	    }

            // Step 3: update positions

	    if(project_every != 0 && ((k % project_every) == 0)) {
		for(index_t v=0; v<nb_pts; ++v) {
		    double* p = mesh_grob()->vertices.point_ptr(v);
		    p[0] = centroids[v].x;
		    p[1] = centroids[v].y;
		    p[2] = centroids[v].z;
		}
	    }

            for(index_t v=0; v<nb_pts; ++v) {
                Geom::mesh_vertex_ref(*mesh_grob(),v) += tau*V[v];
            }


	    if(save_every != 0 && ((k % save_every) == 0)) {


		std::string iter_str = String::to_string(k + first_iter);
		while(iter_str.length() < 5) {
		    iter_str = "0" + iter_str;
		}

		if(verbose) {
		    Logger::out("Euler")
			<< "Saving timestep: " << iter_str << std::endl;
		}
		scene_graph()->save_current_object(
		    "Euler_timestep_" + iter_str + ".graphite"
		);
	    }

            omega->update();
            mesh_grob()->update();
        }
    }


    void MeshGrobTransportCommands::Euler_on_surface(
        const MeshGrobName& omega_name,
        double tau,
        double epsilon,
        double g,
	index_t nb_iter,
	bool compute_RVD,
	bool verbose,
	index_t project_every,
	bool physical
    ) {
        MeshGrob* omega = MeshGrob::find(scene_graph(),omega_name);
        if(omega == nullptr) {
            Logger::err("Euler") << omega_name << ": no such MeshGrob"
                                 << std::endl;
            return;
        }

        if(omega->facets.nb() == 0) {
            Logger::err("Euler") << omega_name << ": does not have any facet"
                                 << std::endl;
            return ;
        }

        index_t nb_pts = mesh_grob()->vertices.nb();

        Attribute<double> m(mesh_grob()->vertices.attributes(),"mass");
        Attribute<vec3> V(mesh_grob()->vertices.attributes(),"V");
        vector<vec3> centroids(nb_pts);

        // CmdLine::set_arg("algo:predicates","exact");
        // CmdLine::set_arg("dbg:delaunay_benchmark",true);

	MeshGrob* RVD = nullptr;
	bool RVD_existed = false;
	if(compute_RVD) {
	    RVD_existed = (MeshGrob::find(scene_graph(),"RVD") != nullptr);
	    RVD = MeshGrob::find_or_create(scene_graph(),"RVD");
	}

        for(unsigned int k=1; k<=nb_iter; ++k) {

	    if(verbose) {
		Logger::out("Euler")
		    << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> "
		    << "Time step = " << k << std::endl;
	    }

            // Step 1: get Laguerre cells centroids

	    compute_Laguerre_centroids_on_surface(
		omega, nb_pts,
		mesh_grob()->vertices.point_ptr(0),
		centroids[0].data(),
		RVD,
		verbose
	    );

            // Step 2: update speeds

            double inveps2 = 1.0 / geo_sqr(epsilon);
	    if(physical) {
		for(index_t v=0; v<nb_pts; ++v) {
		    V[v] += tau*(
			(inveps2 / m[v]) *
			(centroids[v] - Geom::mesh_vertex(*mesh_grob(),v)) +
			vec3(0.0, 0.0, -g)
		    );
		}
	    } else {
		for(index_t v=0; v<nb_pts; ++v) {
		    V[v] += tau*(
			inveps2*(
			    centroids[v] - Geom::mesh_vertex(*mesh_grob(),v)
			) + m[v] * vec3(0.0, 0.0, -g)
		    );
		}
	    }

            // Step 3: update positions

	    if(project_every != 0 && ((k % project_every) == 0)) {
		for(index_t v=0; v<nb_pts; ++v) {
		    double* p = mesh_grob()->vertices.point_ptr(v);
		    p[0] = centroids[v].x;
		    p[1] = centroids[v].y;
		    p[2] = centroids[v].z;
		}
	    }


            for(index_t v=0; v<nb_pts; ++v) {
                Geom::mesh_vertex_ref(*mesh_grob(),v) += tau*V[v];
            }

            omega->update();
            mesh_grob()->update();

	    if(compute_RVD) {

		Attribute<index_t> f_chart(RVD->facets.attributes(), "chart");
		Attribute<double> f_mass(RVD->facets.attributes(), "mass");
		FOR(f,RVD->facets.nb()) {
		    f_mass[f] = m[f_chart[f]];
		}

		if(!RVD_existed) {
		    mesh_grob()->set_visible(false);
		    RVD->get_shader()->set_property("painting","ATTRIBUTE");
		    RVD->get_shader()->set_property("attribute","facets.mass");
		    RVD->get_shader()->set_property(
			"colormap","blue_red;true;0;false;false"
		    );
		    RVD->get_shader()->invoke_method("autorange");
		    RVD->set_visible(true);
		    omega->set_visible(false);
		}

		RVD->update();
	    }

	    // Need to trigger a graphics update (not needed in verbose
	    // mode since message displays trigger graphics updates).
	    if(!verbose) {
		mesh_grob()->redraw();
	    }
        }
    }

    void MeshGrobTransportCommands::smooth_interface() {
	vector<vec3>
	    facet_center(mesh_grob()->facets.nb(), vec3(0.0, 0.0, 0.0));

	vector<vec3>
	    vertex_center(mesh_grob()->vertices.nb(), vec3(0.0, 0.0, 0.0));

	vector<index_t> vertex_degree(mesh_grob()->vertices.nb(),0);
	FOR(f, mesh_grob()->facets.nb()) {
	    FOR(lv, mesh_grob()->facets.nb_vertices(f)) {
		index_t v = mesh_grob()->facets.vertex(f,lv);
		++vertex_degree[v];
		facet_center[f] += vec3(mesh_grob()->vertices.point_ptr(v));
	    }
	}
	FOR(f, mesh_grob()->facets.nb()) {
	    facet_center[f] *= 1.0 / double(mesh_grob()->facets.nb_vertices(f));
	}
	FOR(f, mesh_grob()->facets.nb()) {
	    FOR(lv, mesh_grob()->facets.nb_vertices(f)) {
		index_t v = mesh_grob()->facets.vertex(f,lv);
		vertex_center[v] += facet_center[f];
	    }
	}
	FOR(v, mesh_grob()->vertices.nb()) {
	    vertex_center[v] =
		1.0 / double(vertex_degree[v]) * vertex_center[v];
	}
	FOR(v, mesh_grob()->vertices.nb()) {
	    mesh_grob()->vertices.point_ptr(v)[0] = vertex_center[v].x;
	    mesh_grob()->vertices.point_ptr(v)[1] = vertex_center[v].y;
	    mesh_grob()->vertices.point_ptr(v)[2] = vertex_center[v].z;
	}
	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::compute_dual_surface_mesh(
	const NewMeshGrobName& dual_name, bool triangulate
    ) {
	MeshGrob* dual = MeshGrob::find_or_create(scene_graph(), dual_name);
	dual->clear();
	dual->vertices.set_dimension(3);
	dual->vertices.create_vertices(mesh_grob()->facets.nb());
	FOR(v,dual->vertices.nb()) {
	    dual->vertices.point_ptr(v)[0]=0.0;
	    dual->vertices.point_ptr(v)[1]=0.0;
	    dual->vertices.point_ptr(v)[2]=0.0;
	}
	FOR(f,mesh_grob()->facets.nb()) {
	    double s = 1.0 / double(mesh_grob()->facets.nb_vertices(f));
	    FOR(lv, mesh_grob()->facets.nb_vertices(f)) {
		index_t v = mesh_grob()->facets.vertex(f,lv);
		dual->vertices.point_ptr(f)[0] += s*mesh_grob()->vertices.point_ptr(v)[0];
		dual->vertices.point_ptr(f)[1] += s*mesh_grob()->vertices.point_ptr(v)[1];
		dual->vertices.point_ptr(f)[2] += s*mesh_grob()->vertices.point_ptr(v)[2];
	    }
	}
	vector<index_t> v2f(mesh_grob()->vertices.nb(),index_t(-1));
	FOR(f,mesh_grob()->facets.nb()) {
	    FOR(lv, mesh_grob()->facets.nb_vertices(f)) {
		index_t v = mesh_grob()->facets.vertex(f,lv);
		v2f[v] = f;
	    }
	}
	FOR(v,mesh_grob()->vertices.nb()) {
	    vector<index_t> star;
	    index_t first_f = v2f[v];
	    index_t first_e = mesh_grob()->facets.find_vertex(first_f,v);
	    index_t f = first_f;
	    index_t e = first_e;
	    do {
		star.push_back(f);
		f = mesh_grob()->facets.adjacent(f,e);
		if(f == NO_FACET) {
		    break;
		}
		e = mesh_grob()->facets.find_vertex(f,v);
	    } while(f != first_f || e != first_e);
	    if(triangulate) {
		for(index_t i=1; i+1<star.size(); ++i) {
		    dual->facets.create_triangle(star[0], star[i], star[i+1]);
		}
	    } else {
		dual->facets.create_polygon(star);
	    }
	}
	dual->facets.connect();
	dual->update();
    }


    inline void write_vec(FILE* F, const double* V) {
	float xyz[3];
	xyz[0] = float(V[0]);
	xyz[1] = float(V[1]);
	xyz[2] = float(V[2]);
	fwrite(xyz, sizeof(float), 3, F);
    }

    void MeshGrobTransportCommands::export_to_Cyril_Crassin_raw(
	const std::string& basename
    ) {
	FOR(f, mesh_grob()->facets.nb()) {
	    if(mesh_grob()->facets.nb_vertices(f) != 3) {
		Logger::err("OTM") << "Mesh is not triangulated"
				   << std::endl;
		return;
	    }
	}
	vector<vec3> vertex_normal(mesh_grob()->vertices.nb(), vec3(0.0, 0.0, 0.0));
	FOR(f, mesh_grob()->facets.nb()) {
	    vec3 N = Geom::mesh_facet_normal(*mesh_grob(), f);
	    FOR(lv, mesh_grob()->facets.nb_vertices(f)) {
		index_t v = mesh_grob()->facets.vertex(f,lv);
		vertex_normal[v]-=N;
	    }
	}
	FOR(v, mesh_grob()->vertices.nb()) {
	    vertex_normal[v] = normalize(vertex_normal[v]);
	}

	{
	    FILE* F = fopen((basename + "_pos.raw").c_str(),"wb");
	    FOR(v, mesh_grob()->vertices.nb()) {
		write_vec(F, mesh_grob()->vertices.point_ptr(v));
	    }
	    fclose(F);
	}

	{
	    FILE* F = fopen((basename + "_normal.raw").c_str(),"wb");
	    FOR(v, mesh_grob()->vertices.nb()) {
		write_vec(F, vertex_normal[v].data());
	    }
	    fclose(F);
	}

	{
	    FILE* F = fopen((basename + "_triangles.raw").c_str(),"wb");
	    fwrite(
		mesh_grob()->facet_corners.vertex_index_ptr(0),
		sizeof(index_t),
		mesh_grob()->facets.nb()*3,
		F
	    );
	    fclose(F);
	}
    }

    inline double wrap_coord(double c, bool do_it) {
	double result = c;
	if(do_it) {
	    while(result < 0.0) {
		result += 1.0;
	    }
	    while(result > 1.0) {
		result -= 1.0;
	    }
	}
	return result;
    }

    void MeshGrobTransportCommands::extract_initial_and_final(
	const NewMeshGrobName& now_name,
	const NewMeshGrobName& initial_name,
	bool wrap_coords,
	bool correct_origin,
	index_t nb_per_axis
    ) {
	if(mesh_grob()->vertices.dimension() != 6) {
	    Logger::err("OTM") << "Current mesh does not contain any animation data"
			       << std::endl;
	    return;
	}

	MeshGrob* now = MeshGrob::find_or_create(scene_graph(), now_name);
	now->clear();
	now->vertices.set_dimension(3);
	now->vertices.create_vertices(mesh_grob()->vertices.nb());

	MeshGrob* initial = MeshGrob::find_or_create(scene_graph(), initial_name);
	initial->clear();
	initial->vertices.set_dimension(3);
	initial->vertices.create_vertices(mesh_grob()->vertices.nb());

	double Tx = 0.0;
	double Ty = 0.0;
	double Tz = 0.0;

	if(correct_origin) {
	    const double* p0 = mesh_grob()->vertices.point_ptr(0) + 3;
//	    double ofs = 1.0 / (pow(double(mesh_grob()->vertices.nb()), 1.0 / 3.0) + 1.0);
	    double ofs = 1.0 / (double(nb_per_axis) + 1.0);
	    Tx = ofs - p0[0];
	    Ty = ofs - p0[1];
	    Tz = ofs - p0[2];
	}

	FOR(v, mesh_grob()->vertices.nb()) {
	    const double* from = mesh_grob()->vertices.point_ptr(v);
	    double* to = now->vertices.point_ptr(v);
	    to[0] = wrap_coord(from[0]+Tx, wrap_coords);
	    to[1] = wrap_coord(from[1]+Ty, wrap_coords);
	    to[2] = wrap_coord(from[2]+Tz, wrap_coords);
	}

	FOR(v, mesh_grob()->vertices.nb()) {
	    const double* from = mesh_grob()->vertices.point_ptr(v) + 3;
	    double* to = initial->vertices.point_ptr(v);
	    to[0] = wrap_coord(from[0]+Tx, wrap_coords);
	    to[1] = wrap_coord(from[1]+Ty, wrap_coords);
	    to[2] = wrap_coord(from[2]+Tz, wrap_coords);
	}

	now->update();
	initial->update();
    }

    void MeshGrobTransportCommands::copy_nearest_point_colors(
	const MeshGrobName& from_name
    ) {
	MeshGrob* from = MeshGrob::find(scene_graph(), from_name);
	if(from == nullptr) {
	    Logger::err("Mesh") << from_name << " no such object"
				<< std::endl;
	    return;
	}

	Attribute<double> from_color;
	from_color.bind_if_is_defined(from->vertices.attributes(), "color");
	if(!from_color.is_bound() || from_color.dimension() != 3) {
	    Logger::err("Mesh") << "Missing color in source object"
				<< std::endl;
	    return;
	}

	Attribute<double> to_color;
	to_color.bind_if_is_defined(
	    mesh_grob()->vertices.attributes(), "color"
	);
	if(!to_color.is_bound()) {
	    to_color.create_vector_attribute(
		mesh_grob()->vertices.attributes(), "color", 3
	    );
	}

	Attribute<double> from_tex_coord;
	from_tex_coord.bind_if_is_defined(
	    from->vertices.attributes(), "tex_coord"
	);

	Attribute<double> to_tex_coord;
	if(from_tex_coord.is_bound() && mesh_grob()->facet_corners.nb() != 0) {
	    to_tex_coord.bind_if_is_defined(
		mesh_grob()->facet_corners.attributes(), "tex_coord"
	    );
	    if(!to_tex_coord.is_bound()) {
		to_tex_coord.create_vector_attribute(
		    mesh_grob()->facet_corners.attributes(), "tex_coord", 2
		);
	    }
	}

	NearestNeighborSearch_var NN = new BalancedKdTree(3);
	NN->set_points(
	    from->vertices.nb(), from->vertices.point_ptr(0),
	    from->vertices.dimension()
	);

	parallel_for(
	    0, mesh_grob()->vertices.nb(),
	    [&NN, &from_color, &to_color, this](index_t v) {
		double sq_dist;
		index_t nearest;
		NN->get_nearest_neighbors(
		    1,
		    mesh_grob()->vertices.point_ptr(v),
		    &nearest,
		    &sq_dist
		);

		to_color[3*v  ] = from_color[3*nearest  ];
		to_color[3*v+1] = from_color[3*nearest+1];
		to_color[3*v+2] = from_color[3*nearest+2];
	    }
	);

	if(from_tex_coord.is_bound() && to_tex_coord.is_bound()) {
	    parallel_for(
		0, mesh_grob()->facet_corners.nb(),
		[&NN, &from_tex_coord, &to_tex_coord, this](index_t c) {
		    index_t v = mesh_grob()->facet_corners.vertex(c);
		    double sq_dist;
		    index_t nearest;
		    NN->get_nearest_neighbors(
			1,
			mesh_grob()->vertices.point_ptr(v),
			&nearest,
			&sq_dist
		    );
		    to_tex_coord[2*c] = from_tex_coord[2*nearest];
		    to_tex_coord[2*c+1] = from_tex_coord[2*nearest+1];
		}
	    );
	}

	mesh_grob()->update();
    }


    void MeshGrobTransportCommands::copy_point_colors(
	const MeshGrobName& from_name
    ) {
	MeshGrob* from = MeshGrob::find(scene_graph(), from_name);
	if(from == nullptr) {
	    Logger::err("Mesh") << from_name << " no such object"
				<< std::endl;
	    return;
	}

	if(from->vertices.nb() != mesh_grob()->vertices.nb()) {
	    Logger::err("Mesh")
		<< "source and target need to have the same nb of vertices"
		<< std::endl;
	    return;
	}

	Attribute<double> from_color;
	from_color.bind_if_is_defined(from->vertices.attributes(), "color");
	if(!from_color.is_bound() || from_color.dimension() != 3) {
	    Logger::err("Mesh") << "Missing color in source object"
				<< std::endl;
	    return;
	}

	Attribute<double> to_color;
	to_color.bind_if_is_defined(
	    mesh_grob()->vertices.attributes(), "color"
	);
	if(!to_color.is_bound()) {
	    to_color.create_vector_attribute(
		mesh_grob()->vertices.attributes(), "color", 3
	    );
	}

	Attribute<double> from_tex_coord;
	from_tex_coord.bind_if_is_defined(
	    from->vertices.attributes(), "tex_coord"
	);
	Attribute<double> to_tex_coord;
	if(from_tex_coord.is_bound()) {
	    to_tex_coord.bind_if_is_defined(
		mesh_grob()->vertices.attributes(), "tex_coord"
	    );
	    if(!to_tex_coord.is_bound()) {
		to_tex_coord.create_vector_attribute(
		    mesh_grob()->vertices.attributes(), "tex_coord", 2
		);
	    }
	}

	for(index_t v: mesh_grob()->vertices) {
	    to_color[3*v  ] = from_color[3*v];
	    to_color[3*v+1] = from_color[3*v+1];
	    to_color[3*v+2] = from_color[3*v+2];
	    if(from_tex_coord.is_bound() && to_tex_coord.is_bound()) {
		to_tex_coord[2*v]   = from_tex_coord[2*v];
		to_tex_coord[2*v+1] = from_tex_coord[2*v+1];
	    }
	}

	mesh_grob()->update();
    }


    void MeshGrobTransportCommands::sample_regions(
	const std::string& points_name,
	index_t total_nb_points,
	double region1_mass,
	double region2_mass,
	double region3_mass,
	double region4_mass,
	double region5_mass
    ) {
	if(mesh_grob()->cells.nb() == 0) {
	    Logger::err("OTM") << "Mesh has no tetrahedron"
			       << std::endl;
	    return;
	}
	Attribute<index_t> region;
	region.bind_if_is_defined(mesh_grob()->cells.attributes(), "region");
	if(!region.is_bound()) {
	    Logger::err("OTM") << "Mesh has no region attribute"
			       << std::endl;
	    return;
	}

	MeshGrob* points = MeshGrob::find_or_create(scene_graph(), points_name);
	points->clear();
	Attribute<double> mass(points->vertices.attributes(),"mass");

	vector<double> masses;

	masses.push_back(0.0);
	masses.push_back(region1_mass);
	masses.push_back(region2_mass);
	masses.push_back(region3_mass);
	masses.push_back(region4_mass);
	masses.push_back(region5_mass);

	vector<double> volumes(masses.size(), 0.0);
	double total_volume = 0.0;

	FOR(t, mesh_grob()->vertices.nb()) {
	    index_t rgn = region[t];
	    if(rgn < volumes.size()) {
		double V = mesh_cell_volume(*mesh_grob(), t);
		volumes[rgn] += V;
		total_volume += V;
	    }
	}

	FOR(rgn, masses.size()) {
	    if(masses[rgn] != 0.0 && volumes[rgn] != 0.0) {

		// Copy region into a mesh.
		Mesh region_mesh;
		region_mesh.vertices.assign_points(
		    mesh_grob()->vertices.point_ptr(0),
		    mesh_grob()->vertices.dimension(),
		    mesh_grob()->vertices.nb()
		);
		FOR(t, mesh_grob()->cells.nb()) {
		    if(region[t] == rgn) {
			region_mesh.cells.create_tet(
			    mesh_grob()->cells.vertex(t,0),
			    mesh_grob()->cells.vertex(t,1),
			    mesh_grob()->cells.vertex(t,2),
			    mesh_grob()->cells.vertex(t,3)
			);
		    }
		}
		mesh_grob()->vertices.remove_isolated();
		mesh_grob()->cells.connect();

		// Sample the region.

		index_t rgn_nb_points = index_t(
		    double(total_nb_points) * volumes[rgn] / total_volume
		);
		CentroidalVoronoiTesselation CVT(&region_mesh, 0, "NN");
		CVT.set_volumetric(true);
		sample(
		    CVT, rgn_nb_points, false, false, false, 0.0, nullptr
		);

		index_t offset = points->vertices.nb();
		points->vertices.create_vertices(rgn_nb_points);
		Memory::copy(
		    points->vertices.point_ptr(offset),
		    CVT.embedding(0),
		    rgn_nb_points*points->vertices.dimension()*sizeof(double)
		);
		FOR(v,rgn_nb_points) {
		    mass[v + offset] = masses[rgn];
		}
	    }
	}

	points->update();
    }

    void MeshGrobTransportCommands::advect(
	velocity_field_t field,
	double t0,
	double dt,
	double nb_timesteps,
	time_integrator_t integrator,
	bool save_timesteps
    ) {
        double t = t0;
	VelocityField_var VF = VelocityField::create(field);
	FOR(i, nb_timesteps) {
	    FOR(v, mesh_grob()->vertices.nb()) {
		double* p = mesh_grob()->vertices.point_ptr(v);
		vec3 V;
		time_integrator(t, dt, vec3(p), V, integrator, VF);
		p[0] += dt * V.x;
		p[1] += dt * V.y;
		p[2] += dt * V.z;
	    }
	    mesh_grob()->update();
	    Logger::out("Advect") << "Timestep: " << t << std::endl;
	    if(save_timesteps) {
	      std::string i_as_string = String::to_string(i);
	      while(i_as_string.length() < 4) {
		i_as_string = "0" + i_as_string;
	      }
	      mesh_grob()->save("advect_" + i_as_string + ".xyz");
	    }
	    t += dt;
	}
    }


    void MeshGrobTransportCommands::select_chart(
	const std::string& chart_attribute_name,
	const std::string& selection_name,
	index_t chart_id
    ) {
	Attribute<index_t> chart;
	chart.bind_if_is_defined(
	    mesh_grob()->facets.attributes(), chart_attribute_name
	);
	if(!chart.is_bound()) {
	    Logger::err("OTM") << chart_attribute_name << ": no such facet attribute"
			       << std::endl;
	    return;
	}
	Attribute<bool> selection(
	    mesh_grob()->facets.attributes(), selection_name
	);
	FOR(f, mesh_grob()->facets.nb()) {
	    selection[f] = (chart[f] == chart_id);
	}
	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::translate(double tx, double ty, double tz) {
	FOR(v, mesh_grob()->vertices.nb()) {
	    double* p = mesh_grob()->vertices.point_ptr(v);
	    p[0] += tx;
	    p[1] += ty;
	    p[2] += tz;
	}
	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::EUR_normalize_periodic_coordinates() {
	FOR(v, mesh_grob()->vertices.nb()) {
	    double* p = mesh_grob()->vertices.point_ptr(v);
	    p[0] = wrap_coord(p[0], true);
	    p[1] = wrap_coord(p[1], true);
	    p[2] = wrap_coord(p[2], true);
	}
	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::EUR_scatter_plot(
	const NewFileName& filename, index_t nb_per_axis
    ) {
	index_t N = index_t(pow(double(mesh_grob()->vertices.nb()), 1.0 / 3.0)+0.5);
	if(N*N*N != mesh_grob()->vertices.nb()) {
	    Logger::err("EUR") << "Number of vertices: " << N << " is not a cube"
			       << std::endl;
	    return;
	}

	index_t skip = nb_per_axis / N;
	if(skip * N != nb_per_axis) {
	    Logger::err("EUR") << "Number of vertices per coord is not a divider of "
			       << nb_per_axis
			       << std::endl;
	    return;
	}

	double Tx = 0.0;
	double Ty = 0.0;
	double Tz = 0.0;
	index_t ofsind =
	    (mesh_grob()->vertices.dimension() == 6) ? 3 : 0;
	const double* p0 = mesh_grob()->vertices.point_ptr(0) + ofsind;
	double ofs = 1.0 / (double(nb_per_axis) + 1.0);
	Tx = ofs - p0[0];
	Ty = ofs - p0[1];
	Tz = ofs - p0[2];

	std::ofstream out(std::string(filename).c_str());

	std::ofstream test_pts("test_pts.xyz");
	std::ofstream test_pts2("test_pts2.xyz");

	FOR(v, mesh_grob()->vertices.nb()) {
	    index_t U = v % N;
	    index_t V = (v / N) % N;
	    index_t W = (v / (N*N)) % N;
	    U *= skip;
	    V *= skip;
	    W *= skip;

	    double Qx = U / double(nb_per_axis) + ofs;
	    double Qy = V / double(nb_per_axis) + ofs;
	    double Qz = W / double(nb_per_axis) + ofs;

	    double* P = mesh_grob()->vertices.point_ptr(v);
	    double Px = P[0] + Tx;
	    double Py = P[1] + Ty;
	    double Pz = P[2] + Tz;

	    out << Qx << " " << Px << std::endl;
	    out << Qy << " " << Py << std::endl;
//	    out << Qz << " " << Pz << std::endl;

	    test_pts2 << Px << " " << Py << " " << Pz << std::endl;
	    test_pts  << Qx << " " << Qy << " " << Qz << std::endl;
	}

    }

    void MeshGrobTransportCommands::copy_attribute_to_geometry(
	const std::string attribute
    ) {
	Attribute<double> attr;
	attr.bind_if_is_defined(mesh_grob()->vertices.attributes(), attribute);
	if(!attr.is_bound()) {
	    Logger::err("copy_attrib")
		<< attribute
		<< " no such vertex attribute" << std::endl;
	    return;
	}
	if(attr.dimension() < 3) {
	    Logger::err("copy_attrib")
		<< attribute
		<< " is not a 3d or more vector" << std::endl;
	    return;
	}
	FOR(v, mesh_grob()->vertices.nb()) {
	    mesh_grob()->vertices.point_ptr(v)[0] = attr[attr.dimension()*v];
	    mesh_grob()->vertices.point_ptr(v)[1] = attr[attr.dimension()*v+1];
	    mesh_grob()->vertices.point_ptr(v)[2] = attr[attr.dimension()*v+2];
	}
	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::save_normals() {
	std::ofstream out("normals.c");
	out << "const int NB=" << mesh_grob()->facets.nb() << ";" << std::endl;
	out << "const double SData[NB*3]={" << std::endl;
	for(index_t f:mesh_grob()->facets) {
	    vec3 N = normalize(Geom::mesh_facet_normal(*mesh_grob(), f));
	    out << "   " << N.x << "," << N.y << "," << N.z;
	    if(f != mesh_grob()->facets.nb()-1) {
		out << ",";
	    }
	    out << std::endl;
	}
	out << "};" << std::endl;
	Logger::out("OT") << "Saved to normals.c" << std::endl;
    }

    void MeshGrobTransportCommands::show_Hilbert_curve(index_t dimension) {
	if(dimension != 2 && dimension != 3) {
	    Logger::err("Hilbert") << "invalid dimension."
				   << std::endl;
	    return;
	}
	mesh_grob()->edges.clear();
	vector<index_t> sorted(mesh_grob()->vertices.nb());
	for(index_t i=0; i<mesh_grob()->vertices.nb(); ++i) {
	    sorted[i] = i;
	}
	compute_Hilbert_order(
	    mesh_grob()->vertices.nb(),
	    mesh_grob()->vertices.point_ptr(0),
	    sorted,
	    0, mesh_grob()->vertices.nb(),
	    dimension,
	    mesh_grob()->vertices.dimension()
	);
	for(index_t i=0; i<sorted.size()-1; ++i) {
	    mesh_grob()->edges.create_edge(sorted[i], sorted[i+1]);
	}
	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::export_points_and_attribute(
         const std::string& attribute_name,
         const NewFileName& file_name
    ) {
       Attribute<double> attrib;
       attrib.bind_if_is_defined(mesh_grob()->vertices.attributes(), attribute_name);
       if(!attrib.is_bound()) {
	  Logger::err("Export") << attribute_name << ": no such vertices attribute"
	                        << std::endl;
	  return;
       }
       std::ofstream out(std::string(file_name).c_str());
       for(index_t i=0; i<mesh_grob()->vertices.nb(); ++i) {
	  const double* p = mesh_grob()->vertices.point_ptr(i);
	  out << p[0] << ' ' << p[1] << ' ' << p[2] << ' ' << attrib[i] << std::endl;
       }
    }


    void MeshGrobTransportCommands::normalize_transported_volume() {
        if(mesh_grob()->vertices.dimension() != 6) {
	    Logger::err("OT") << "Mesh does not have transport" << std::endl;
	    return;
        }
        double vol1 = 0.0;
        double vol2 = 0.0;
	vec3 center1(0.0, 0.0, 0.0);
	vec3 center2(0.0, 0.0, 0.0);
	vec3 origin(0.0, 0.0, 0.0);
	for(index_t f : mesh_grob()->facets) {
	    index_t i = mesh_grob()->facets.vertex(f,0);
	    index_t j = mesh_grob()->facets.vertex(f,1);
	    index_t k = mesh_grob()->facets.vertex(f,2);
	    vec3 p1(mesh_grob()->vertices.point_ptr(i));
	    vec3 p2(mesh_grob()->vertices.point_ptr(j));
	    vec3 p3(mesh_grob()->vertices.point_ptr(k));
	    vec3 q1(mesh_grob()->vertices.point_ptr(i)+3);
	    vec3 q2(mesh_grob()->vertices.point_ptr(j)+3);
	    vec3 q3(mesh_grob()->vertices.point_ptr(k)+3);
	    double V1 = Geom::tetra_signed_volume(origin, p1, p2, p3);
	    double V2 = Geom::tetra_signed_volume(origin, q1, q2, q3);
	    vol1 += V1;
	    vol2 += V2;
	    center1 += (V1 / 4.0)*(p1+p2+p3);
	    center2 += (V2 / 4.0)*(q1+q2+q3);
	}
	center1 = (1.0/vol1)*center1;
	center2 = (1.0/vol2)*center2;
	vol1 = ::fabs(vol1);
	vol2 = ::fabs(vol2);
	double s = pow(vol1/vol2,1.0/3.0);
	for(index_t v : mesh_grob()->vertices) {
	    vec3 p(mesh_grob()->vertices.point_ptr(v)+3);
	    p = p-center2;
	    p = s*p;
	    p = p+center2;
	    mesh_grob()->vertices.point_ptr(v)[3] = p.x;
	    mesh_grob()->vertices.point_ptr(v)[4] = p.y;
	    mesh_grob()->vertices.point_ptr(v)[5] = p.z;
	}
	mesh_grob()->update();
    }
}

namespace {
    using namespace OGF;

    void get_facet_rings(
	const MeshGrob* M, index_t t_seed, vector<index_t>& N, index_t nb_rings
    ) {
	N.resize(0);
	std::set<index_t> visited;
	N.push_back(t_seed);
	visited.insert(t_seed);
	for(index_t k=0; k<nb_rings; ++k) {
	    for(index_t t: N) {
		for(index_t e=0; e<M->facets.nb_vertices(t); ++e) {
		    index_t neigh = M->facets.adjacent(t,e);
		    if(
			neigh != index_t(-1) &&
			visited.find(neigh) == visited.end()
		    ) {
			visited.insert(neigh);
			N.push_back(neigh);
		    }
		}
	    }
	}
    }

}

namespace OGF {

    void MeshGrobTransportCommands::inflate(
	const MeshGrobName& points_name, double R0, double R1, index_t nb_rings
    ) {
	MeshGrob* points = MeshGrob::find(scene_graph(),points_name);
	if(points == nullptr) {
	    Logger::err("Transport") << points_name << ": no such MeshGrob"
				     << std::endl;
	    return;
	}

	MeshFacetsAABB AABB(*mesh_grob());

	vector<vec3> V(mesh_grob()->vertices.nb());
	vector<double> d(mesh_grob()->vertices.nb(), R0);
	for(index_t i : mesh_grob()->vertices) {
	    V[i] = normalize(vec3(mesh_grob()->vertices.point_ptr(i)));
	}

	for(index_t i : points->vertices) {
	    vec3 D(points->vertices.point_ptr(i));
	    MeshFacetsAABB::Intersection I;
	    vector<index_t> N;
	    if(AABB.ray_nearest_intersection(Ray(vec3(0.0, 0.0, 0.0), D),I)) {
		get_facet_rings(mesh_grob(), I.f, N, nb_rings);
		for(index_t f: N) {
		    for(index_t lv=0;
			lv<mesh_grob()->facets.nb_vertices(f); ++lv) {
			index_t v = mesh_grob()->facets.vertex(f, lv);
			d[v] = std::max(d[v], dot(D,V[v]));
		    }
		}
	    }
	}

	for(index_t i : mesh_grob()->vertices) {
	    vec3 p = (d[i] + R1) * V[i];
	    mesh_grob()->vertices.point_ptr(i)[0] = p.x;
	    mesh_grob()->vertices.point_ptr(i)[1] = p.y;
	    mesh_grob()->vertices.point_ptr(i)[2] = p.z;
	}

	mesh_grob()->update();
    }

    void MeshGrobTransportCommands::extract_trajectories() {
        if(mesh_grob()->vertices.dimension() != 6) {
            Logger::err("Transport") << "Mesh has no trajectory data"
                                     << std::endl;
            return;
        }
        MeshGrob* trajectories =
            MeshGrob::find_or_create(scene_graph(),"trajectories");
        trajectories->clear();
        trajectories->vertices.set_dimension(3);
        for(index_t v: mesh_grob()->vertices) {
            vec3 p1(mesh_grob()->vertices.point_ptr(v));
            vec3 p2(mesh_grob()->vertices.point_ptr(v)+3);
            trajectories->vertices.create_vertex(p1.data());
            trajectories->vertices.create_vertex(p2.data());
            trajectories->edges.create_edge(
                trajectories->vertices.nb()-2,
                trajectories->vertices.nb()-1
            );
        }
        trajectories->update();
    }
}

/************************************************************************/


namespace OGF {
    void MeshGrobTransportCommands::load_Hydra(const FileName& filename) {

        mesh_grob()->clear();

        try {
            HydraFile in(filename);

            in.load_header();

            Logger::out("Hydra") << String::format("version %f", in.version())
                                 << std::endl;

            index_t NPART = in.nb_particles();
            Logger::out("Hydra") << String::format(" - irun    =%d",in.ibuf2.d.irun)     << std::endl;
            Logger::out("Hydra") << String::format(" - nobj    =%d",in.ibuf2.d.nobj)     << std::endl;
            Logger::out("Hydra") << String::format(" - ngas    =%d",in.ibuf2.d.ngas)     << std::endl;
            Logger::out("Hydra") << String::format(" - ndark   =%d",in.ibuf2.d.ndark)    << std::endl;
            Logger::out("Hydra") << String::format(" - h100    =%f",in.ibuf2.d.h100)     << std::endl;
            Logger::out("Hydra") << String::format(" - box100  =%f",in.ibuf2.d.box100)   << std::endl;
            Logger::out("Hydra") << String::format(" - tstart  =%f",in.ibuf2.d.tstart)   << std::endl;
            Logger::out("Hydra") << String::format(" - omega0  =%f",in.ibuf2.d.omega0)   << std::endl;
            Logger::out("Hydra") << String::format(" - xlambda0=%f",in.ibuf2.d.xlambda0) << std::endl;
            Logger::out("Hydra") << String::format(" - h0t0    =%f",in.ibuf2.d.h0t0)     << std::endl;

            Logger::out("Hydra") << String::format(" - itime =%d",in.ibuf1.d.itime)  << std::endl;
            Logger::out("Hydra") << String::format(" - itstop=%d",in.ibuf1.d.itstop) << std::endl;
            Logger::out("Hydra") << String::format(" - itdump=%d",in.ibuf1.d.itdump) << std::endl;
            Logger::out("Hydra") << String::format(" - itout =%d",in.ibuf1.d.itout)  << std::endl;

            Logger::out("Hydra") << String::format(" -  time =%f",in.ibuf1.d.time)   << std::endl;
            Logger::out("Hydra") << String::format(" - atime =%f",in.ibuf1.d.atime)  << std::endl;
            Logger::out("Hydra") << String::format(" - htime =%f",in.ibuf1.d.htime)  << std::endl;
            Logger::out("Hydra") << String::format(" - dtime =%f",in.ibuf1.d.dtime)  << std::endl;

            Logger::out("Hydra") << String::format(" - tstop =%f",in.ibuf1.d.tstop)  << std::endl;
            Logger::out("Hydra") << String::format(" - tout  =%f",in.ibuf1.d.tout)   << std::endl;
            Logger::out("Hydra") << String::format(" - icdump=%d",in.ibuf1.d.icdump) << std::endl;

            Logger::out("Hydra") <<  " ---> nb particles=" << NPART << std::endl;
            mesh_grob()->vertices.set_dimension(3);
            mesh_grob()->vertices.create_vertices(NPART);

            in.skip_itype();
            in.skip_rm();

            Logger::out("Hydra") << "read r" << std::endl;
            in.begin_record();
            for(index_t i=0; i<NPART; ++i) {
                float p[3];
                in.read(p);
                for(index_t coord=0; coord<3; ++coord) {
                    mesh_grob()->vertices.point_ptr(i)[coord] = double(p[coord]);
                }
            }
            in.end_record();

            in.skip_v();

        } catch(std::logic_error& err) {
            Logger::err("Hydra") << err.what() << std::endl;
            return;
        }

        if(mesh_grob()->get_shader() != nullptr) {
            mesh_grob()->set_shader("Cosmo");
        }
    }

    void MeshGrobTransportCommands::load_binary(const FileName& filename) {
        try {
            FILE* f = fopen(std::string(filename).c_str(),"rb");
            if(f == nullptr) {
                throw(std::logic_error(
                          "Could not open " + std::string(filename)
                ));
            }

            fseek(f, 0L, SEEK_END);
            size_t filesize = size_t(ftell(f));
            rewind(f);

            if(filesize % 12 != 0) {
                throw(std::logic_error("Invalid file size"));
            }

            index_t N = index_t(filesize/12);

            mesh_grob()->clear();
            mesh_grob()->vertices.set_dimension(3);
            mesh_grob()->vertices.create_vertices(N);
            for(index_t i=0; i<N; ++i) {
                float xyz[3];
                if(fread(xyz, sizeof(xyz), 1, f) != 1) {
                    throw(std::logic_error("Error while reading file"));
                }
                mesh_grob()->vertices.point_ptr(i)[0] = double(xyz[0]);
                mesh_grob()->vertices.point_ptr(i)[1] = double(xyz[1]);
                mesh_grob()->vertices.point_ptr(i)[2] = double(xyz[2]);
            }

            fclose(f);
        } catch (std::logic_error& err) {
            Logger::err("Cosmo") << err.what() << std::endl;
            return;
        }

        if(mesh_grob()->get_shader() != nullptr) {
            mesh_grob()->set_shader("Cosmo");
        }
    }

    void MeshGrobTransportCommands::load_Gaia(
        const FileName& filename, double Vscale, double ortho_scale
    ) {
        mesh_grob()->clear();
        mesh_grob()->vertices.set_dimension(3);

        Attribute<double> V0;
        Attribute<double> V1;
        Attribute<double> V2;

        V0.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV0");
        V1.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV1");
        V2.bind_if_is_defined(mesh_grob()->vertices.attributes(), "eigenV2");
        if(
            (V0.is_bound() && V0.dimension() != 3) ||
            (V1.is_bound() && V1.dimension() != 3) ||
            (V2.is_bound() && V2.dimension() != 3)
        ) {
            Logger::err("Cosmo") << "V0 or V1 or V2 already exist with wrong dim"
                                 << std::endl;
            return;
        }

        if(!V0.is_bound()) {
            V0.create_vector_attribute(
                mesh_grob()->vertices.attributes(), "eigenV0",3
            );
        }

        if(!V1.is_bound()) {
            V1.create_vector_attribute(
                mesh_grob()->vertices.attributes(), "eigenV1",3
            );
        }

        if(!V2.is_bound()) {
            V2.create_vector_attribute(
                mesh_grob()->vertices.attributes(), "eigenV2",3
            );
        }

        LineInput in(filename);
        if(!in.OK()) {
            Logger::err("Cosmo") << "Could not open " << filename << std::endl;
            return;
        }
        try {
            while( !in.eof() && in.get_line() ) {
                in.get_fields();
                vec3 p(
                    in.field_as_double(0),
                    in.field_as_double(1),
                    in.field_as_double(2)
                );
                vec3 V(
                    in.field_as_double(3),
                    in.field_as_double(4),
                    in.field_as_double(5)
                );
                V = Vscale * V;
                double l = length(V);
                vec3 X = ortho_scale * l * normalize(Geom::perpendicular(V));
                vec3 Y = ortho_scale * l * normalize(cross(V,X));

                index_t v = mesh_grob()->vertices.create_vertex(p.data());
                V0[3*v  ] = V.x;
                V0[3*v+1] = V.y;
                V0[3*v+2] = V.z;

                V1[3*v  ] = X.x;
                V1[3*v+1] = X.y;
                V1[3*v+2] = X.z;

                V2[3*v  ] = Y.x;
                V2[3*v+1] = Y.y;
                V2[3*v+2] = Y.z;
            }
        } catch(const std::logic_error& ex) {
            Logger::err("Cosmo") << ex.what() << std::endl;
        }

        mesh_grob()->update();
    }

    void MeshGrobTransportCommands::save_binary(const NewFileName& filename) {
        try {
            FILE* f = fopen(std::string(filename).c_str(),"wb");
            if(f == nullptr) {
                throw(std::logic_error(
                          "Could not create file:" + std::string(filename)
                ));
            }
            for(index_t i: mesh_grob()->vertices) {
                float xyz[3];
                xyz[0] = float(mesh_grob()->vertices.point_ptr(i)[0]);
                xyz[1] = float(mesh_grob()->vertices.point_ptr(i)[1]);
                xyz[2] = float(mesh_grob()->vertices.point_ptr(i)[2]);
                if(fwrite(xyz, sizeof(xyz), 1, f) != 1) {
                    throw(std::logic_error("Error while writing file"));
                }
            }
            fclose(f);
        } catch (std::logic_error& err) {
            Logger::err("Cosmo") << err.what() << std::endl;
        }
    }



    void MeshGrobTransportCommands::create_box() {
        MeshGrob* box = MeshGrob::find_or_create(scene_graph(), "box");
        box->clear();

        double x1 = 0.0;
        double y1 = 0.0;
        double z1 = 0.0;

        double x2 = 1.0;
        double y2 = 1.0;
        double z2 = 1.0;

        Object* shdr = mesh_grob()->get_shader();
        if(shdr != nullptr) {
            auto update_bound = [&](
                double& bound, const std::string& prop_name
            ) {
                std::string prop_val;
                if(!(shdr->get_property(prop_name, prop_val))) {
                    return;
                }
                try {
                    int val = String::to_int(prop_val);
                    std::cerr << "val =" << std::endl;
                    bound = double(val)/100.0;
                } catch(...) {
                    return;
                }
            };
            update_bound(x1, "minx");
            update_bound(y1, "miny");
            update_bound(z1, "minz");
            update_bound(x2, "maxx");
            update_bound(y2, "maxy");
            update_bound(z2, "maxz");

        }

        index_t v0 = box->vertices.create_vertex(vec3(x1,y1,z1).data());
        index_t v1 = box->vertices.create_vertex(vec3(x1,y1,z2).data());
        index_t v2 = box->vertices.create_vertex(vec3(x1,y2,z1).data());
        index_t v3 = box->vertices.create_vertex(vec3(x1,y2,z2).data());
        index_t v4 = box->vertices.create_vertex(vec3(x2,y1,z1).data());
        index_t v5 = box->vertices.create_vertex(vec3(x2,y1,z2).data());
        index_t v6 = box->vertices.create_vertex(vec3(x2,y2,z1).data());
        index_t v7 = box->vertices.create_vertex(vec3(x2,y2,z2).data());

        box->facets.create_quad(v3,v7,v6,v2);
        box->facets.create_quad(v0,v1,v3,v2);
        box->facets.create_quad(v1,v5,v7,v3);
        box->facets.create_quad(v5,v4,v6,v7);
        box->facets.create_quad(v0,v4,v5,v1);
        box->facets.create_quad(v2,v6,v4,v0);

        box->facets.connect();


        Object* shd = box->get_shader();
        if(shd != nullptr) {
            shd->set_property("surface_style", "true;0 0 0 0");
            shd->set_property("mesh_style", "true;1 1 1;3");
            shd->set_property("culling_mode","CULL_BACK");
        }
    }


    void MeshGrobTransportCommands::load_Calabi_Yau(const FileName& filename) {
        Attribute<double> CY;
        CY.bind_if_is_defined(mesh_grob()->vertices.attributes(), "CY");
        if(!CY.is_bound()) {
            CY.create_vector_attribute(
                mesh_grob()->vertices.attributes(), "CY", 12
            );
        }
        try {
            FILE* f = fopen(std::string(filename).c_str(),"rb");
            if(f == nullptr) {
                throw(std::logic_error(
                          "Could not open " + std::string(filename)
                ));
            }

            fseek(f, 0L, SEEK_END);
            size_t filesize = size_t(ftell(f));
            rewind(f);

            if(filesize % 48 != 0) {
                throw(std::logic_error("Invalid file size"));
            }

            index_t N = index_t(filesize/48);

            mesh_grob()->clear();
            mesh_grob()->vertices.set_dimension(3);
            mesh_grob()->vertices.create_vertices(N);
            for(index_t i=0; i<N; ++i) {
                float xyz[12];
                if(fread(xyz, sizeof(xyz), 1, f) != 1) {
                    throw(std::logic_error("Error while reading file"));
                }
                mesh_grob()->vertices.point_ptr(i)[0] = double(xyz[0]);
                mesh_grob()->vertices.point_ptr(i)[1] = double(xyz[1]);
                mesh_grob()->vertices.point_ptr(i)[2] = double(xyz[2]);
                for(index_t c=0; c<12; ++c) {
                    CY[i*12+c] = xyz[c];
                }
            }
            fclose(f);
        } catch (std::logic_error& err) {
            Logger::err("Cosmo") << err.what() << std::endl;
            return;
        }
        mesh_grob()->get_shader()->set_property("vertices_style","true; 0 1 0 1; 1");
    }

    void MeshGrobTransportCommands::show_Calabi_Yau_coordinates(index_t x, index_t y, index_t z) {
        if(x >= 12  || y >= 12 || z >= 12) {
            Logger::err("Cosmo") << "CY coords should be in 0..11" << std::endl;
            return;
        }
        Attribute<double> CY;
        CY.bind_if_is_defined(mesh_grob()->vertices.attributes(), "CY");
        if(!CY.is_bound()  || CY.dimension() != 12) {
            Logger::err("Cosmo") << "Missing or invalid CY attribute" << std::endl;
            return;
        }
        for(index_t v: mesh_grob()->vertices) {
            mesh_grob()->vertices.point_ptr(v)[0] = CY[12*v+x];
            mesh_grob()->vertices.point_ptr(v)[1] = CY[12*v+y];
            mesh_grob()->vertices.point_ptr(v)[2] = CY[12*v+z];
        }
        mesh_grob()->update();
    }


    void MeshGrobTransportCommands::split_Calabi_Yau() {
        Attribute<double> CY;
        CY.bind_if_is_defined(mesh_grob()->vertices.attributes(), "CY");
        if(!CY.is_bound()  || CY.dimension() != 12) {
            Logger::err("Cosmo") << "Missing or invalid CY attribute" << std::endl;
            return;
        }

        MeshGrob* split = MeshGrob::find_or_create(scene_graph(),"split");
        split->clear();

        Attribute<double> theta(split->vertices.attributes(),"theta");
        Attribute<double> phi(split->vertices.attributes(),"phi");

        split->vertices.create_vertices(mesh_grob()->vertices.nb()*4);
        for(index_t v: mesh_grob()->vertices) {

            // Take one that has regular sampling
            double x = CY[12*v+3 ];
            double y = CY[12*v+4 ];
            double z = CY[12*v+5 ];
            double vtheta = atan2(y,x);
            double vphi = atan2(sqrt(x*x+y*y),z);

            split->vertices.point_ptr(4*v+0)[0] = CY[12*v+0 ] - 2.0;
            split->vertices.point_ptr(4*v+0)[1] = CY[12*v+1 ] - 2.0;
            split->vertices.point_ptr(4*v+0)[2] = CY[12*v+2 ];
            theta[4*v] = vtheta;
            phi[4*v]   = vphi;


            split->vertices.point_ptr(4*v+1)[0] = CY[12*v+3 ] - 2.0;
            split->vertices.point_ptr(4*v+1)[1] = CY[12*v+4 ] + 2.0;
            split->vertices.point_ptr(4*v+1)[2] = CY[12*v+5 ];
            theta[4*v+1] = vtheta;
            phi[4*v+1]   = vphi;


            split->vertices.point_ptr(4*v+2)[0] = CY[12*v+6 ] + 2.0;
            split->vertices.point_ptr(4*v+2)[1] = CY[12*v+7 ] - 2.0;
            split->vertices.point_ptr(4*v+2)[2] = CY[12*v+8 ];
            theta[4*v+2] = vtheta;
            phi[4*v+2]   = vphi;

            split->vertices.point_ptr(4*v+3)[0] = CY[12*v+9 ] + 2.0;
            split->vertices.point_ptr(4*v+3)[1] = CY[12*v+10] + 2.0;
            split->vertices.point_ptr(4*v+3)[2] = CY[12*v+11];
            theta[4*v+3] = vtheta;
            phi[4*v+3]   = vphi;

        }

        split->get_shader()->set_property("vertices_style","true; 0 1 0 1; 1");
        split->update();
    }

    /**************************************************************/

    index_t copy_cell(
        Mesh* target, Mesh* source, index_t i, double shrink, bool flip, bool edges
    ) {
        Attribute<index_t> chart(source->facets.attributes(),"chart");

        std::map<index_t, index_t> nxt;
        index_t first_v = NO_INDEX;
        for(index_t f: source->facets) {
            if(chart[f] != i) {
                continue;
            }
            index_t N = source->facets.nb_vertices(f);
            for(index_t le=0; le<N; ++le) {
                index_t g = source->facets.adjacent(f,le);
                if(g == NO_INDEX || chart[g] != i) {
                    index_t v1 = source->facets.vertex(f,le);
                    index_t v2 = source->facets.vertex(f,(le + 1) % N);
                    nxt[v1] = v2;
                    first_v = v1;
                }
            }
        }

        vec3 G(0.0, 0.0, 0.0);
        index_t v = first_v;
        do {
            G += vec3(source->vertices.point_ptr(v));
            v = nxt[v];
        } while(v != first_v);
        G = (1.0/double(nxt.size()))*G;

        index_t newf = target->facets.create_polygon(index_t(nxt.size()));
        v = first_v;
        index_t lv = 0;
        index_t N = index_t(nxt.size());
        do {
            vec3 p = shrink*G+(1.0-shrink)*vec3(source->vertices.point_ptr(v));
            // p.z = z;
            target->facets.set_vertex(
                newf,
                flip ? N-1-lv : lv,
                target->vertices.create_vertex(p.data())
            );
            v = nxt[v];
            ++lv;
        } while(v != first_v);

        if(edges) {
            for(index_t lv=0; lv<N; ++lv) {
                index_t v1 = target->facets.vertex(newf, lv);
                index_t v2 = target->facets.vertex(newf, (lv+1)%N);
                target->edges.create_edge(v1,v2);
            }
        }
        return newf;
    }

    vec3 mesh_facet_centroid(const Mesh& M, index_t f) {
        vec3 mg(0.0, 0.0, 0.0);
        double m=0.0;
        const double* p0 = M.vertices.point_ptr(
            M.facet_corners.vertex(M.facets.corners_begin(f))
        );
        for(
            index_t i = M.facets.corners_begin(f) + 1;
            i + 1 < M.facets.corners_end(f); i++
        ) {
            const double* p1 = M.vertices.point_ptr(M.facet_corners.vertex(i));
            const double* p2 = M.vertices.point_ptr(M.facet_corners.vertex(i + 1));
            double A = GEO::Geom::triangle_area(
                p0, p1, p2, coord_index_t(3)
            );
            mg += A*vec3(p0);
            mg += A*vec3(p1);
            mg += A*vec3(p2);
            m += 3.0*A;
        }
        return (1.0/m)*mg;
    }

    void connect(Mesh* M, index_t f1, index_t f2, const std::string& name) {

        double z1 = M->vertices.point_ptr(M->facets.vertex(f1,0))[2];
        double z2 = M->vertices.point_ptr(M->facets.vertex(f2,0))[2];
        double delta_z = z2 - z1;

        index_t N1 = M->facets.nb_vertices(f1);
        index_t N2 = M->facets.nb_vertices(f2);
        double R1 = 0.0;
        double R2 = 0.0;
        vec3 g1 = mesh_facet_centroid(*M,f1);
        vec3 g2 = mesh_facet_centroid(*M,f2);
        for(index_t lv=0; lv<N1; ++lv) {
            index_t v = M->facets.vertex(f1,lv);
            R1 = std::max(R1, length(vec3(M->vertices.point_ptr(v)) - g1));
        }
        for(index_t lv=0; lv<N2; ++lv) {
            index_t v = M->facets.vertex(f2,lv);
            R2 = std::max(R2, length(vec3(M->vertices.point_ptr(v)) - g2));
        }
        double R = std::max(R1,R2);
        CDT2d CDT;
        CDT.set_delaunay(true);
        double D = 20.0*R;

        vector<index_t> cdt2mesh;

        CDT.create_enclosing_rectangle(-D,-D,D,D);

        cdt2mesh.push_back(NO_INDEX);
        cdt2mesh.push_back(NO_INDEX);
        cdt2mesh.push_back(NO_INDEX);
        cdt2mesh.push_back(NO_INDEX);

        index_t base1 = CDT.nv();
        for(index_t lv = 0; lv<N1; ++lv) {
            index_t v = M->facets.vertex(f1,lv);
            vec3 p = vec3(M->vertices.point_ptr(v))-g1;
            CDT.insert(vec2(p.x, p.y));
            cdt2mesh.push_back(v);
        }
        index_t base2 = CDT.nv();
        for(index_t lv = 0; lv<N2; ++lv) {
            index_t v = M->facets.vertex(f2,lv);
            vec3 p = 2.0 * (vec3(M->vertices.point_ptr(v))-g2);
            CDT.insert(vec2(p.x, p.y));
            cdt2mesh.push_back(v);
        }

        for(index_t lv = 0; lv<N1; ++lv) {
            CDT.insert_constraint(base1+lv, base1+(lv+1)%N1);
        }

        for(index_t lv = 0; lv<N2; ++lv) {
            CDT.insert_constraint(base2+lv, base2+(lv+1)%N2);
        }

        for(index_t t=0; t<CDT.nT(); ++t) {
            index_t v[3] = { NO_INDEX, NO_INDEX, NO_INDEX } ;
            index_t nb1 = 0;
            index_t nb2 = 0;
            for(index_t lv=0; lv<3; ++lv) {
                v[lv] = CDT.Tv(t,lv);
                if(v[lv] >= base2) {
                    ++nb2;
                } else if(v[lv] >= base1) {
                    ++nb1;
                }
                if(v[lv] < cdt2mesh.size()) {
                    v[lv] = cdt2mesh[v[lv]];
                } else {
                    v[lv] = NO_INDEX;
                }
            }

            if(
                v[0] != NO_INDEX && v[1] != NO_INDEX && v[2] != NO_INDEX &&
                nb1 > 0 && nb2 > 0
            ) {
                M->facets.create_triangle(v[0],v[1],v[2]);
                for(index_t lv1=0; lv1<3; ++lv1) {
                    index_t lv2=(lv1+1)%3;
                    const double* p1 = M->vertices.point_ptr(v[lv1]);
                    const double* p2 = M->vertices.point_ptr(v[lv2]);
                    double d = Geom::distance(p1,p2,2);
                    if((p1[2] != p2[2]) && (d < delta_z) && (v[lv1] < v[lv2])) {
                        M->edges.create_edge(v[lv1],v[lv2]);
                    }
                }
            }

        }
        if(name != "") {
            CDT.save(name);
        }
    }

    void MeshGrobTransportCommands::get_path_bundles(
        const std::string& format,
        double shrink,
        index_t max_timestep,
        index_t skip
    ) {
        index_t N = 0;

        std::vector<MeshGrob*> timesteps;
        for(index_t i=1; i<max_timestep; i += (skip+1)) {
            std::string name = String::format(format.c_str(), i);
            MeshGrob* M = MeshGrob::find(scene_graph(), name);
            if(M != nullptr) {
                timesteps.push_back(M);
            } else {
                break;
            }
            if(N == 0) {
                Attribute<index_t> chart(M->facets.attributes(),"chart");
                for(index_t f: M->facets) {
                    N = std::max(N, chart[f]+1);
                }
                Logger::out("OT") << "N = " << N << std::endl;
            }
        }

        mesh_grob()->clear();

        if(timesteps.size() == 0) {
            Logger::err("OTM") << "Did not find any timestep"
                               << std::endl;
            return;
        }

        Attribute<index_t> to_delete(mesh_grob()->facets.attributes(),"to_delete");

        for(index_t i=0; i<N; ++i) {
            bool flip = false;
            bool edges = true;
            index_t f1 = copy_cell(
                mesh_grob(), timesteps[0], i, shrink, flip, edges
            );
            std::string name = String::format("debugCDT_%03d.geogram",i);
            name = "";
            for(index_t timestep=1; timestep < timesteps.size(); ++timestep) {
                MeshGrob* M = timesteps[timestep];
                flip = true;
                edges = (timestep == timesteps.size()-1);
                index_t f2 = copy_cell(
                    mesh_grob(), M, i, shrink, flip, edges
                );
                connect(mesh_grob(), f1, f2, name);
                name = "";
                if(timestep != timesteps.size()-1) {
                    to_delete[f2] = 1;
                }
                f1 = f2;
            }
        }

        vector<index_t> to_delete_bkp = to_delete.get_vector();
        mesh_grob()->facets.delete_elements(to_delete_bkp);
        mesh_grob()->facets.connect();

        mesh_grob()->update();
    }
}
