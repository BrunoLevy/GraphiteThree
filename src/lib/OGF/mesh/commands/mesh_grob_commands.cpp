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
 

#include <OGF/mesh/commands/mesh_grob_commands.h>

namespace OGF {
    MeshGrobCommands::MeshGrobCommands() { 
    }
        
    MeshGrobCommands::~MeshGrobCommands() { 
    }        


    void MeshGrobCommands::show_attribute(
	const std::string& attribute_name, MeshGrob* M
    ) {
	if(M == nullptr) {
	    M = mesh_grob();
	}
	
	Shader* shader = M->get_shader();

	if(shader == nullptr) {
	    return;
	}


	if(
	    !shader->has_property("painting") ||
	    !shader->has_property("attribute")
	) {
	    return;
	}
	
	std::string shd_painting;
	std::string shd_attribute;
	shader->get_property("painting",shd_painting);
	shader->get_property("attribute",shd_attribute);
	
	bool first_time = (
	    shd_painting != "ATTRIBUTE" ||
	    shd_attribute != attribute_name
	);
	
	shader->set_property("painting","ATTRIBUTE");
	shader->set_property("attribute", attribute_name);

	if(first_time) {
	    shader->invoke_method("autorange");
	    if(!String::string_starts_with(attribute_name,"cells.")) {
		shader->set_property("lighting","false");
	    }
	    shader->set_property("colormap","plasma;true;0;false;false;");
	}
    }

    void MeshGrobCommands::show_charts() {
	show_attribute("facets.chart");
	Shader* shader = mesh_grob()->get_shader();
	shader->invoke_method("autorange");
	shader->set_property("colormap","random;true;65537;false;false;");
    }
    
    void MeshGrobCommands::show_mesh(MeshGrob* M) {
	
	if(M == nullptr) {
	    M = mesh_grob();
	}
	
	Shader* shader = M->get_shader();

	if(shader == nullptr) {
	    return;
	}

	if(!shader->has_property("mesh_style")) {
	    return;
	}

	std::string shd_mesh;
	std::vector<std::string> shd_mesh_fields;

	shader->get_property("mesh_style",shd_mesh);
	String::split_string(shd_mesh, ';', shd_mesh_fields, false);

	if(shd_mesh_fields.size() != 3) {
	    return;
	}

	if(shd_mesh_fields[0] == "true") {
	    return;
	}
	
	shd_mesh = "true;"+shd_mesh_fields[1]+";"+shd_mesh_fields[2];
	shader->set_property("mesh_style", shd_mesh);
    }

    void MeshGrobCommands::show_UV(
	const std::string& UV_prop_name, MeshGrob* M 
    ) {
	if(M == nullptr) {
	    M = mesh_grob();
	}
	
	Shader* shader = M->get_shader();

	if(shader == nullptr) {
	    return;
	}

	if(
	    !shader->has_property("painting") ||
	    !shader->has_property("tex_coords")	||
	    !shader->has_property("lighting")	
	) {
	    return;
	}

	shader->set_property("painting","TEXTURE");
	shader->set_property("tex_image","");
	shader->set_property("tex_coords",UV_prop_name);
	shader->set_property("normal_map","false");
	shader->set_property("tex_repeat","10");
	shader->set_property("lighting","true");        
    }

    void MeshGrobCommands::show_colors(
	const std::string& attribute, MeshGrob* M
    ) {
	if(M == nullptr) {
	    M = mesh_grob();
	}
	
	Shader* shader = M->get_shader();

	if(shader == nullptr) {
	    return;
	}

	if(!shader->has_property("painting")) {
	    return;
	}

	shader->set_property("painting","COLOR");
	shader->set_property("colors",attribute);
    }

}

