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
	const std::string& attribute_name
    ) {
	Shader* shader = mesh_grob()->get_shader();

	if(shader == nullptr) {
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
	    shader->set_property("lighting","false");
	    shader->set_property("colormap","inferno;true;0;false;false;");
	}
    }
    
}

