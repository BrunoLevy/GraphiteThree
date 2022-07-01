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
 
#include <OGF/WarpDrive/commands/mesh_grob_martingale_commands.h>

#include <exploragram/optimal_transport/sampling.h>
#include <exploragram/optimal_transport/optimal_transport_3d.h>
#include <exploragram/optimal_transport/optimal_transport_2d.h>
#include <exploragram/optimal_transport/optimal_transport_on_surface.h>

#include <OGF/scene_graph/types/scene_graph.h>

#include <geogram/basic/command_line.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_sampling.h>
#include <geogram/basic/stopwatch.h>

namespace {
    using namespace OGF;

    /**
     * \brief Tests whether a mesh is 2D or 3D.
     * \param[in] mesh a pointer to the mesh to be tested.
     * \retval true if the mesh is 2D
     * \retval false otherwise
     */
    bool mesh_is_flat(const MeshGrob* mesh) {
	if(mesh->cells.nb() != 0) {
	    return false;
	}
	if(mesh->vertices.dimension() ==2) {
	    return true;
	}
	FOR(i, mesh->vertices.nb()) {
	    if(mesh->vertices.point_ptr(i)[2] != 0.0) {
		return false;
	    }
	}
	return true;
    }

    
    // Note: we could re-implement all Euler code using this function.
    
    /**
     * \brief Computes semi-discrete optimal transport
     *   between a pointset and a surface.
     * \details If the surface is flat, it uses OptimalTransportMap2d, else
     *   it uses OptimalTransportMapOnSurface. If the surface has a "weight"
     *   attribute, then it is used as the source measure. If the pointset has
     *   a "nu" attribute, then it is used as the target measure.
     * \param[in] omega the domain
     * \param[in] points the pointset.
     * \param[out] centroids where to store the centroids, or nullptr.
     * \param[out] laguerre where to store the Laguerre diagram, or nullptr.
     * \param[in] exact_predicates if set, force using exact predicates
     */
    void semi_discrete_OT(
	MeshGrob* omega,
	MeshGrob* points,
	MeshGrob* centroids,
	MeshGrob* laguerre,
	bool exact_predicates
    ) {
	
	std::string predicates_mode_backup =
	    CmdLine::get_arg("algo:predicates");
	    
	if(exact_predicates) {
	    CmdLine::set_arg("algo:predicates","exact");
	}

	
	bool mesh_is_2D = mesh_is_flat(omega);
	
	Attribute<double> nu;
	nu.bind_if_is_defined(points->vertices.attributes(),"nu");
	
	OptimalTransportMap* OTM = nullptr;
	if(mesh_is_2D) {
	    OTM = new OptimalTransportMap2d(omega);
	} else {
	    omega->vertices.set_dimension(4);
	    omega->update();
	    if(omega->cells.nb() != 0) {
		OTM = new OptimalTransportMap3d(omega);
	    } else {
		OTM = new OptimalTransportMapOnSurface(omega);
	    }
	}
	    
	OTM->set_points(
	    points->vertices.nb(),
	    points->vertices.point_ptr(0),
	    points->vertices.dimension()
	);
	
	if(nu.is_bound()) {
	    FOR(i,points->vertices.nb()) {
		OTM->set_nu(i,nu[i]);
	    }
	}

	OTM->set_verbose(!mesh_is_2D);
	OTM->set_regularization(0.0);	
	OTM->set_epsilon(1e-3);
	OTM->set_Newton(true);
	OTM->optimize(1000);

	if(laguerre != nullptr) {
	    laguerre->clear();
	    OTM->get_RVD(*laguerre);
	    laguerre->update();
	}

	if(centroids != nullptr) {
	    if(mesh_is_2D) {
		vector<double> p_centroids(points->vertices.nb()*2);	  
		OTM->compute_Laguerre_centroids(p_centroids.data());
		centroids->vertices.assign_points(p_centroids, 2, true);
		centroids->vertices.set_dimension(3);
		centroids->update();
	    } else {
		vector<double> p_centroids(points->vertices.nb()*3);
		OTM->compute_Laguerre_centroids(p_centroids.data());
		centroids->vertices.assign_points(p_centroids, 3, true);
		centroids->update();
	    }
	}
	
	if(!mesh_is_2D) {
	    omega->vertices.set_dimension(3);
	    omega->update();
	}

	delete OTM;

	if(exact_predicates) {
	    CmdLine::set_arg("algo:predicates", predicates_mode_backup);
	}
    }
}


namespace OGF {

    MeshGrobMartingaleCommands::MeshGrobMartingaleCommands() { 
    }
        
    MeshGrobMartingaleCommands::~MeshGrobMartingaleCommands() { 
    }        

    void MeshGrobMartingaleCommands::init_martingale(
	const NewMeshGrobName& points_name,
	const NewMeshGrobName& laguerre_name,
	const NewMeshGrobName& centroids_name,
	index_t nb_points,
	bool use_exact_predicates
    ) {
	if(mesh_grob()->facets.nb() == 0) {
	    Logger::err("OTM") << "Current mesh has no facet" << std::endl;
	    return;
	}
	
	MeshGrob* points = MeshGrob::find_or_create(
	    scene_graph(), points_name
	);
	
	MeshGrob* laguerre = MeshGrob::find_or_create(
	    scene_graph(), laguerre_name
	);

	MeshGrob* centroids = MeshGrob::find_or_create(
	    scene_graph(), centroids_name
	);
	
	if(nb_points != 0) {
	    points->clear();
	    points->vertices.set_dimension(3);
	    points->vertices.create_vertices(nb_points);
	    Attribute<double> dummy;
	    if(mesh_grob()->cells.nb() != 0) {
		mesh_generate_random_samples_in_volume<3>(
		    *mesh_grob(),
		    points->vertices.point_ptr(0),
		    nb_points,
		    dummy
		);
	    } else {
		mesh_generate_random_samples_on_surface<3>(
		    *mesh_grob(),
		    points->vertices.point_ptr(0),
		    nb_points,
		    dummy
		);
	    }
	} 
	points->update();
	
	semi_discrete_OT(
	    mesh_grob(), points, centroids, laguerre, use_exact_predicates
	);
	
        if(points->get_shader() != nullptr) {
            points->get_shader()->set_property(
                "vertices_style", "true;0 1 0 1;2"
	    );
        }

        if(centroids->get_shader() != nullptr) {
            centroids->get_shader()->set_property(
                "vertices_style", "true;1 0 0 1;2"
	    );
        }

	if(laguerre->get_shader() != nullptr) {
	    laguerre->get_shader()->set_property(
		"attributes", "true"					       
	    );
	    if(mesh_grob()->cells.nb() != 0) {
		laguerre->get_shader()->set_property(
		    "attribute", "cells.region"
		);
	    } else {
		laguerre->get_shader()->set_property(
		    "attribute", "facets.chart"
		);
	    }
	    laguerre->get_shader()->set_property(
		"attribute_max",
		String::to_string(points->vertices.nb()-1)
	    );
	    laguerre->get_shader()->set_property(
		"border_style", "false;0 0 0.5 1;2"
	    );
	}

	if(mesh_grob()->get_shader() != nullptr) {
	    mesh_grob()->get_shader()->set_property(
		"surface_style", "false; 0.5 0.5 0.5 1.0"
	    );
	}
    }

  
    void MeshGrobMartingaleCommands::recenter_martingale(
        const MeshGrobName& points_name,
	const MeshGrobName& laguerre_name,
	const MeshGrobName& centroids_name,
	index_t nb_iter,
	bool use_exact_predicates
    ) {

	if(mesh_grob()->facets.nb() == 0) {
	    Logger::err("OTM") << "Current mesh has no facet" << std::endl;
	    return;
	}
	
	MeshGrob* points = MeshGrob::find(
	    scene_graph(), points_name
	);

	if(points == nullptr) {
	    Logger::err("OTM") << points_name
			       << ": no such object" << std::endl;
	    return;	    
	}
	
	MeshGrob* laguerre = MeshGrob::find(
	    scene_graph(), laguerre_name
	);

	if(laguerre == nullptr) {
	    Logger::err("OTM") << laguerre_name
			       << ": no such object" << std::endl;
	    return;	    
	}
	
	MeshGrob* centroids = MeshGrob::find(
	    scene_graph(), centroids_name
	);

	if(centroids == nullptr) {
	    Logger::err("OTM") << centroids_name
			       << ": no such object" << std::endl;
	    return;
	}
	
	for(index_t i=0; i<nb_iter; ++i) {
	    points->vertices.assign_points(
		centroids->vertices.point_ptr(0),
		3,
		centroids->vertices.nb()
	    );
	    points->update();
	    semi_discrete_OT(
		mesh_grob(), points, centroids, laguerre, use_exact_predicates
	    );
	    centroids->update();
	    laguerre->redraw();
	}
    }
}
