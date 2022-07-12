/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2016 INRIA - Project ALICE
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

#include <OGF/mesh/commands/mesh_grob_spectral_commands.h>

namespace {
    using namespace OGF;


    struct MHStruct {
	Mesh* mesh;
	Attribute<double> attribute;
    };
    
    void mh_callback(
	index_t eigen_index,
	double eigen_val, const double* eigen_vector,
	void* client_data
    ) {
	Logger::out("MH") << eigen_index << ":" << eigen_val << std::endl;
	geo_argused(eigen_val);
	MHStruct* MH = static_cast<MHStruct*>(client_data);
	for(index_t v=0; v<MH->mesh->vertices.nb(); ++v) {
	    MH->attribute[v*MH->attribute.dimension()+eigen_index] = eigen_vector[v];
	}
    }
    
}

namespace OGF {

    MeshGrobSpectralCommands::MeshGrobSpectralCommands() {
    }

    MeshGrobSpectralCommands::~MeshGrobSpectralCommands() {
    }
    
    void MeshGrobSpectralCommands::compute_manifold_harmonics(
	index_t nb_eigens,
	LaplaceBeltramiDiscretization discretization,
	const std::string& attribute,
	double shift,
	index_t nb_eigens_per_band,
	bool print_spectrum
    ) {
	if(nb_eigens_per_band != 0) {
	    MHStruct MH;
	    MH.mesh = mesh_grob();
	    if(
		MH.mesh->vertices.attributes().is_defined(attribute)
	    ) {
		MH.mesh->vertices.attributes().delete_attribute_store(attribute);
	    }
	    MH.attribute.create_vector_attribute(
		MH.mesh->vertices.attributes(), attribute, nb_eigens
	    );
	    mesh_compute_manifold_harmonics_by_bands(
		*mesh_grob(),
		nb_eigens,
		discretization,
		mh_callback,
		nb_eigens_per_band,
		shift,
		&MH
	    );
	} else {
	    mesh_compute_manifold_harmonics(
		*mesh_grob(),
		nb_eigens,
		discretization,
		attribute,
		shift,
		print_spectrum
	    );
	}
	show_attribute("vertices.eigen["+String::to_string(nb_eigens-1)+"]");
	mesh_grob()->update();
    }
}

