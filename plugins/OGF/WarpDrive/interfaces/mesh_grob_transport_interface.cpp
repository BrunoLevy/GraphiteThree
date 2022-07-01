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
 
#include <OGF/WarpDrive/interfaces/mesh_grob_transport_interface.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/scene_graph/NL/vector.h>
#include <OGF/scene_graph/NL/matrix.h>

#include <exploragram/optimal_transport/optimal_transport_2d.h>
#include <exploragram/optimal_transport/optimal_transport_3d.h>
#include <exploragram/optimal_transport/optimal_transport_on_surface.h>

#include <geogram/NL/nl_matrix.h>

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

namespace OGF {

    MeshGrobTransport::MeshGrobTransport() {
    }

    MeshGrobTransport::~MeshGrobTransport() {
    }

    void MeshGrobTransport::compute_optimal_Laguerre_cells_centroids(
	MeshGrob* Omega, NL::Vector* centroids, NL::Vector* weights,
	MeshGrobTransportCommands::EulerMode mode
    ) {
	switch(mode) {
	    case MeshGrobTransportCommands::EULER_2D: {
		centroids->resize(
		    mesh_grob()->vertices.nb(),2,ogf_meta<double>::type()
		);
		vector<double> points_in(mesh_grob()->vertices.nb()*2);
		FOR(v,mesh_grob()->vertices.nb()) {
		    points_in[2*v  ] = mesh_grob()->vertices.point_ptr(v)[0];
		    points_in[2*v+1] = mesh_grob()->vertices.point_ptr(v)[1];
		}
		if(weights != nullptr) {
		    weights->resize(
			mesh_grob()->vertices.nb(), 1, ogf_meta<double>::type()
		    );
		}
		compute_Laguerre_centroids_2d(
		    Omega, mesh_grob()->vertices.nb(),
		    points_in.data(),
		    (double*)(centroids->data()),
		    nullptr, false,
		    0, nullptr, 0, 0.0,
		    nullptr, (weights == nullptr) ? nullptr : (double*)weights->data()
		);
	    } break;
	    case MeshGrobTransportCommands::EULER_3D: {
		if(weights != nullptr) {
		    Logger::err("OTM")
			<< "Weights output not implemented yet for 3D mode"
			<< std::endl;
		}
		centroids->resize(
		    mesh_grob()->vertices.nb(),3,ogf_meta<double>::type()
		);
		compute_Laguerre_centroids_3d(
		    Omega, mesh_grob()->vertices.nb(),
		    mesh_grob()->vertices.point_ptr(0),
		    (double*)(centroids->data())
		);
	    } break;
	    case MeshGrobTransportCommands::EULER_ON_SURFACE: {
		if(weights != nullptr) {
		    Logger::err("OTM")
			<< "Weights output not implemented yet "
			<< "for on-surface mode"
			<< std::endl;
		}
		centroids->resize(
		    mesh_grob()->vertices.nb(),3,ogf_meta<double>::type()
		);
		compute_Laguerre_centroids_on_surface(
		    Omega, mesh_grob()->vertices.nb(),
		    mesh_grob()->vertices.point_ptr(0),
		    (double*)(centroids->data())
		);
	    } break;
	}
    }

    void MeshGrobTransport::compute_Laguerre_cells_measures(
	MeshGrob* Omega,
	NL::Vector* weights,
	NL::Vector* measures,
	MeshGrobTransportCommands::EulerMode mode
    ) {
	compute_Laguerre_cells_P1_Laplacian(
	    Omega, weights, nullptr, measures, mode
	);
    }


    void MeshGrobTransport::compute_Laguerre_cells_P1_Laplacian(
	MeshGrob* Omega,
	NL::Vector* weights_in,
	NL::Matrix* Laplacian_in,
	NL::Vector* measures_in,
	MeshGrobTransportCommands::EulerMode mode
    ) {
	NLMatrix Laplacian = nullptr; 
	if(Laplacian_in != nullptr) {
	    Laplacian = Laplacian_in->implementation();
	    if(Laplacian->m != Laplacian->n) {
		Logger::err("OTM")
		    << "Specified Laplacian is not a square matrix"
		    << std::endl;
		return;
	    }
	    if(Laplacian->n != mesh_grob()->vertices.nb()) {
		Logger::err("OTM")
		    << "Specified Laplacian does not have the correct size"
		    << std::endl;
		return;
	    }
	}
	double* measures = nullptr;
	if(measures_in != nullptr) {
	    if(measures_in->get_element_meta_type()!=ogf_meta<double>::type()) {
		Logger::err("OTM")
		    << "Wrong element type for measures vector"
		    << std::endl;
	    }
	    if(measures_in->get_nb_elements() != mesh_grob()->vertices.nb()) {
		Logger::err("OTM")
		    << "Measure vector does not have the correct size"
		    << std::endl;
		return;
	    }
	    measures = (double*)measures_in->data();
	}
	
	double* weights = nullptr;
	if(weights_in != nullptr) {
	    if(weights_in->get_element_meta_type()!=ogf_meta<double>::type()) {
		Logger::err("OTM") << "Wrong element type for weights vector"
				   << std::endl;
	    }
	    if(weights_in->get_nb_elements() != mesh_grob()->vertices.nb()) {
		Logger::err("OTM")
		    << "Weights vector does not have the correct size"
		    << std::endl;
		return;
	    }
	    weights = (double*)weights_in->data();
	} else {
	    Logger::err("OTM") << "Missing weights vector"
			       << std::endl;
	    return;
	}
	

	switch(mode) {
	    case MeshGrobTransportCommands::EULER_2D: {
		OptimalTransportMap2d OTM(Omega);
		OTM.set_points(
		    mesh_grob()->vertices.nb(),
		    mesh_grob()->vertices.point_ptr(0),
		    mesh_grob()->vertices.dimension()
		);
		OTM.compute_P1_Laplacian(weights, Laplacian, measures);
	    } break;
	    case MeshGrobTransportCommands::EULER_3D: {
		OptimalTransportMap3d OTM(Omega);
		OTM.set_points(
		    mesh_grob()->vertices.nb(),
		    mesh_grob()->vertices.point_ptr(0),
		    mesh_grob()->vertices.dimension()
		);
		OTM.compute_P1_Laplacian(weights, Laplacian, measures);		
	    } break;
	    case MeshGrobTransportCommands::EULER_ON_SURFACE: {
		OptimalTransportMapOnSurface OTM(Omega);
		OTM.set_points(
		    mesh_grob()->vertices.nb(),
		    mesh_grob()->vertices.point_ptr(0),
		    mesh_grob()->vertices.dimension()
		);
		OTM.compute_P1_Laplacian(weights, Laplacian, measures);		
	    } break;
	}
    }
    
    void MeshGrobTransport::compute_Laguerre_diagram(
	MeshGrob* Omega,
	NL::Vector* weights,
	MeshGrob* RVD,
	MeshGrobTransportCommands::EulerMode mode	    
    ) {
	if(
	    weights == nullptr ||
	    weights->nb_elements() != mesh_grob()->vertices.nb() ||
	    weights->get_element_meta_type() != ogf_meta<double>::type()
	) {
	    Logger::err("OTM") << "Invalid weights vector" << std::endl;
	    return;
	}

	RVD->clear();
	RVD->update();
	
	switch(mode) {
	    case MeshGrobTransportCommands::EULER_2D: {
		vector<double> points_in(mesh_grob()->vertices.nb()*2);
		FOR(v,mesh_grob()->vertices.nb()) {
		    points_in[2*v  ] = mesh_grob()->vertices.point_ptr(v)[0];
		    points_in[2*v+1] = mesh_grob()->vertices.point_ptr(v)[1];
		}
		compute_Laguerre_centroids_2d(
		    Omega, mesh_grob()->vertices.nb(),
		    points_in.data(),
		    nullptr,
		    RVD, false,
		    0, nullptr, 0, 0.0,
		    (double*)weights->data(), nullptr, 0
		);
		RVD->update();
	    } break;
	    case MeshGrobTransportCommands::EULER_3D: {
		Logger::err("OTM")
		    << "Not implemented for 3D mode" << std::endl;
	    } break;
	    case MeshGrobTransportCommands::EULER_ON_SURFACE: {
		Logger::err("OTM")
		    << "Not implemented for on-surface mode" << std::endl;
	    } break;
	}
    }

    
    double MeshGrobTransport::Omega_measure(
	MeshGrob* Omega,
	MeshGrobTransportCommands::EulerMode mode	    
    ) {
	double result = 0;
	switch(mode) { // HERE
	    case MeshGrobTransportCommands::EULER_2D: {
		OptimalTransportMap2d OTM(Omega);
		result = OTM.total_mass();		
	    } break;
	    case MeshGrobTransportCommands::EULER_3D: {
		OptimalTransportMap3d OTM(Omega);
		result = OTM.total_mass();		
	    } break;
	    case MeshGrobTransportCommands::EULER_ON_SURFACE: {
		OptimalTransportMapOnSurface OTM(Omega);
		result = OTM.total_mass();		
	    } break;
	}
	return result;
    }
    
    
    
}
