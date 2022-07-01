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
 * As an exception to the GPL, Graphite can be linked with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/mesh/commands/mesh_grob_points_commands.h>
#include <geogram/points/co3ne.h>
#include <geogram/points/kd_tree.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/third_party/PoissonRecon/poisson_geogram.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/basic/progress.h>

namespace OGF {

    MeshGrobPointsCommands::MeshGrobPointsCommands() { 
    }

    MeshGrobPointsCommands::~MeshGrobPointsCommands() { 
    }
    
    void MeshGrobPointsCommands::smooth_point_set(
        unsigned int nb_iterations,        
        unsigned int nb_neighbors
    ) {
        GEO::Co3Ne_smooth(*mesh_grob(), nb_neighbors, nb_iterations);
        mesh_grob()->update();
    }


    void MeshGrobPointsCommands::reconstruct_surface_SSSR(
        double radius,
        unsigned int nb_iterations,
        unsigned int nb_neighbors
    ) {
	mesh_grob()->lock_graphics();
        mesh_grob()->facets.clear();
        double R = bbox_diagonal(*mesh_grob());

        mesh_repair(*mesh_grob(), GEO::MESH_REPAIR_COLOCATE, 1e-6*R);
        
        radius *= 0.01 * R;

        if(nb_iterations != 0) {
            smooth_point_set(nb_iterations, nb_neighbors);
        }
        
        Co3Ne_reconstruct(*mesh_grob(), radius);

        // Note: we could also use the following function,
        //  but it seems to be bugged (TODO)
        // Co3Ne_smooth_and_reconstruct(
        //    *mesh_grob(), radius, nb_neighbors, nb_iterations
        // );

	mesh_grob()->unlock_graphics();	
        mesh_grob()->update();
        
        // Hide the vertices, so that one can see the facets.
        // (note: needs to be done AFTER mesh_grob()->update() else
        //  graphic display is triggered with an incoherent object).
        if(mesh_grob()->get_shader() != nullptr && mesh_grob()->facets.nb() != 0) {
            mesh_grob()->get_shader()->set_property(
                "vertices_style", "false;0 1 0 1;2"
            );
        }
    }


    void MeshGrobPointsCommands::reconstruct_surface_Poisson(
        const NewMeshGrobName& reconstruction_name,
        index_t depth
    ) {
        {
            Attribute<double> normal;
            normal.bind_if_is_defined(
                mesh_grob()->vertices.attributes(), "normal"
            );
            
            if(!normal.is_bound()) {
                Logger::warn("Poisson") << "Missing \'normal\' vertex attribute"
                                       << std::endl;
                Logger::out("Poisson") << "(estimating normals)" << std::endl;
		if(!estimate_normals()) {
		    Logger::warn("Poisson")
			<< "Normals computation canceled" << std::endl;
		    return;
		}
		normal.bind_if_is_defined(
		    mesh_grob()->vertices.attributes(), "normal"
		);
		geo_assert(normal.is_bound());
            }
	    
            if(normal.dimension() != 3) {
                Logger::err("Poisson")
                    << "Wrong dimension for \'normal\' vertex attribute"
                    << std::endl;
                Logger::err("Poisson")
                    << "Expected 3 and got " << normal.dimension()
                    << std::endl;
                return;
            }
        }

        
        MeshGrob* reconstruction =
            MeshGrob::find_or_create(scene_graph(), reconstruction_name);
        reconstruction->clear();

	Logger::out("Poisson") << "Reconstructing..." << std::endl;
        PoissonReconstruction poisson;
        poisson.set_depth(depth);
        poisson.reconstruct(mesh_grob(), reconstruction);
        
        reconstruction->update();
    }

    void MeshGrobPointsCommands::reconstruct_surface_Delaunay2d() {
	mesh_grob()->lock_graphics();
        mesh_grob()->facets.clear();
	Delaunay_var delaunay = Delaunay::create(2,"BDEL2d");
	vector<double> pts(mesh_grob()->vertices.nb()*2);
	for(index_t v: mesh_grob()->vertices) {
	    pts[2*v]   = mesh_grob()->vertices.point_ptr(v)[0];
	    pts[2*v+1] = mesh_grob()->vertices.point_ptr(v)[1];	    
	}
	delaunay->set_vertices(mesh_grob()->vertices.nb(), pts.data());
	Logger::out("Delaunay") << "Created " << delaunay->nb_cells() << " triangles" << std::endl;
	mesh_grob()->facets.create_triangles(delaunay->nb_cells());
	FOR(t, delaunay->nb_cells()) {
	    mesh_grob()->facets.set_vertex(t, 0, index_t(delaunay->cell_vertex(t,0)));
	    mesh_grob()->facets.set_vertex(t, 1, index_t(delaunay->cell_vertex(t,1)));
	    mesh_grob()->facets.set_vertex(t, 2, index_t(delaunay->cell_vertex(t,2)));	    
	}
	mesh_grob()->facets.connect();
	mesh_grob()->unlock_graphics();
	mesh_grob()->update();
    }
    
    bool MeshGrobPointsCommands::estimate_normals(
	index_t nb_neighbors, bool reorient
    ) {
	bool result = Co3Ne_compute_normals(
	    *mesh_grob(), nb_neighbors, reorient
	);
	mesh_grob()->update();
	return result;
    }
    
    void MeshGrobPointsCommands::sample_surface(
        const NewMeshGrobName& points_name,
        bool copy_normals,
        unsigned int nb_points,
        unsigned int Lloyd_iter,
        unsigned int Newton_iter,
        unsigned int Newton_m
    ) {
	if(mesh_grob()->facets.nb() == 0) {
	    Logger::err("Sample") << "Mesh has no facet"
				  << std::endl;
	    return;
	}
	if(!mesh_grob()->facets.are_simplices()) {
	    Logger::err("Sample") << "Mesh facets are not simplices"
				  << std::endl;
	    return;
	}
        mesh_grob()->vertices.set_dimension(3);
        
        MeshGrob* points = MeshGrob::find_or_create(scene_graph(), points_name);
        points->clear();

        CentroidalVoronoiTesselation CVT(mesh_grob());
        CVT.compute_initial_sampling(nb_points);

        if(Lloyd_iter != 0) {
            try {
                ProgressTask progress("Lloyd", 100);
                CVT.set_progress_logger(&progress);
                CVT.Lloyd_iterations(Lloyd_iter);
            }
            catch(const TaskCanceled&) {
            }
        }

        if(Newton_iter != 0) {
            try {
                ProgressTask progress("Newton", 100);
                CVT.set_progress_logger(&progress);
                CVT.Newton_iterations(Newton_iter, Newton_m);                
            }
            catch(const TaskCanceled&) {
            }
        }

        mesh_grob()->update();
        
        points->vertices.assign_points(CVT.embedding(0), 3, CVT.nb_points());

        if(copy_normals) {
            Attribute<double> normal;
	    normal.bind_if_is_defined(
		points->vertices.attributes(), "normal"
	    );
	    if(normal.is_bound() && normal.dimension() != 3) {
		Logger::err("Sample")
	          << "\'normal\' attribute exists but is not a vector attribute"
		  << std::endl;
	    } else {
		if(!normal.is_bound()) {
		    normal.create_vector_attribute(
			points->vertices.attributes(), "normal", 3
		    );
		}
		MeshFacetsAABB AABB(*mesh_grob());
		for(index_t i: points->vertices) {
		    vec3 p(points->vertices.point_ptr(i));
		    vec3 q;
		    double sq_dist;
		    index_t f = AABB.nearest_facet(p,q,sq_dist);
		    vec3 N = normalize(Geom::mesh_facet_normal(*mesh_grob(),f));
		    for(index_t c=0; c<3; ++c) {
			points->vertices.point_ptr(i)[c] = q[c];
			normal[3*i+c] = N[c];
		    }
		}
	    }
        }
        
        points->update();

        // show the vertices.
        // (note: needs to be done AFTER points->update() else
        //  graphic display is triggered with an incoherent object).
        if(points->get_shader() != nullptr && points->vertices.nb() != 0) {
            points->get_shader()->set_property(
                "vertices_style", "true;0 1 0 1;2"
            );
        }
    }

    void MeshGrobPointsCommands::sample_volume(
        const NewMeshGrobName& points_name,
        unsigned int nb_points,
        unsigned int Lloyd_iter,
        unsigned int Newton_iter,
        unsigned int Newton_m
    ) {
        mesh_grob()->vertices.set_dimension(3);
        
        MeshGrob* points = MeshGrob::find_or_create(scene_graph(), points_name);
	if(nb_points != 0) {
	  points->clear();
	}

	if(mesh_grob()->cells.nb() == 0) {
	    if(!mesh_grob()->facets.are_simplices()) {
		Logger::err("CVT") << "Boundary is not triangulated"
				   << std::endl;
		return;
	    }
	    mesh_tetrahedralize(*mesh_grob());
	    mesh_grob()->update();
	}

	if(
	    mesh_grob()->cells.nb() == 0 ||
	    !mesh_grob()->cells.are_simplices()
	) {
	    Logger::err("CVT") << "Mesh is not tetrahedralized" << std::endl;
	    return;
	}

	CentroidalVoronoiTesselation CVT(mesh_grob());
	CVT.set_volumetric(true);
	if(nb_points != 0) {	
	  CVT.compute_initial_sampling(nb_points);
	} else {
	  CVT.set_points(points->vertices.nb(), points->vertices.point_ptr(0));
	}

	if(Lloyd_iter != 0) {
	  try {
	    ProgressTask progress("Lloyd", 100);
	    CVT.set_progress_logger(&progress);
	    CVT.Lloyd_iterations(Lloyd_iter);
	  }
	  catch(const TaskCanceled&) {
	  }
	}

	if(Newton_iter != 0) {
	  try {
	    ProgressTask progress("Newton", 100);
	    CVT.set_progress_logger(&progress);
	    CVT.Newton_iterations(Newton_iter, Newton_m);                
	  }
	  catch(const TaskCanceled&) {
	  }
	}

	mesh_grob()->update();
	points->vertices.assign_points(CVT.embedding(0), 3, CVT.nb_points());
	points->update();

        // show the vertices.
        // (note: needs to be done AFTER points->update() else
        //  graphic display is triggered with an incoherent object).
        if(points->get_shader() != nullptr && points->vertices.nb() != 0) {
            points->get_shader()->set_property(
                "vertices_style", "true;0 1 0 1;2"
            );
        }
    }

    
    void MeshGrobPointsCommands::create_vertex(double x, double y, double z) {
        index_t v = mesh_grob()->vertices.create_vertex();
        double* p = mesh_grob()->vertices.point_ptr(v);
        p[0] = x;
        p[1] = y;
        p[2] = z;
        mesh_grob()->update();
    }

    void MeshGrobPointsCommands::detect_outliers(index_t N, double R) {
	// Remove duplicated vertices
	mesh_repair(*mesh_grob(), GEO::MESH_REPAIR_COLOCATE, 0.0);

	// Compute nearest neighbors using a KdTree.
	NearestNeighborSearch_var NN = new BalancedKdTree(3); // 3 is for 3D
	NN->set_points(mesh_grob()->vertices.nb(), mesh_grob()->vertices.point_ptr(0));
        Attribute<bool> is_outlier(
            mesh_grob()->vertices.attributes(), "selection"
        );
	
	double R2 = R*R; // squared threshold
	// (KD-tree returns squared distances)
       
	// Process all the points in parallel. 
	// The point sequence [0..mesh_grob()->vertices.nb()-1] is split
	// into intervals [from,to[ processed by the lambda
	// function below. There is one interval per processor core,
	// all processed in parallel.
	parallel_for_slice(
	    0,mesh_grob()->vertices.nb(),
	    [this,N,&NN,R2,&is_outlier](index_t from, index_t to) {
		vector<index_t> neigh(N);
		vector<double> neigh_sq_dist(N);
		for(index_t v=from; v<to; ++v) {
		    NN->get_nearest_neighbors(
			N, mesh_grob()->vertices.point_ptr(v),
			neigh.data(), neigh_sq_dist.data()
		    );
		    is_outlier[v] = (neigh_sq_dist[N-1] > R2);
		}
	    }
	);
	mesh_grob()->update();
    }

    
}

