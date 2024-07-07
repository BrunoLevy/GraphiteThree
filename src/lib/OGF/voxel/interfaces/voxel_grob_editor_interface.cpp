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
 *     (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#include <OGF/voxel/interfaces/voxel_grob_editor_interface.h>
#include <OGF/scene_graph/NL/vector.h>
#include <OGF/gom/reflection/meta_type.h>
#include <OGF/gom/reflection/meta.h>

namespace OGF {

    /*************************************************************/
    
    VoxelGrobEditor::VoxelGrobEditor() {
    }

    VoxelGrobEditor::~VoxelGrobEditor() {
    }

    /*******************************************************************/

    void VoxelGrobEditor::update() {
	if(voxel_grob() != nullptr) {
	    voxel_grob()->update();
	}
    }
    
    bool VoxelGrobEditor::check_voxel_grob() const {
	if(voxel_grob() == nullptr) {
	    Logger::err("VoxelGrobEditor") << "No VoxelGrob" << std::endl;
	    return false;
	}
	return true;
    }

    /*******************************************************************/
    
    NL::Vector* VoxelGrobEditor::find_attribute(
	const std::string& attribute_name, bool quiet
    ) {
	AttributesManager& attrmgr = voxel_grob()->attributes();
	
	AttributeStore* attrstore =
	    attrmgr.find_attribute_store(attribute_name);
	
	if(attrstore == nullptr) {
	    if(!quiet) {
		Logger::err("VoxelGrobEditor")
		    << attribute_name << " : no such attribute"
		    << std::endl;
	    }
	    return nullptr;
	}
	return new NL::Vector(voxel_grob(), attrstore);
    }

    NL::Vector* VoxelGrobEditor::create_attribute(
	const std::string& attribute_name,
	index_t dimension, MetaType* type
    ) {
	if(type == nullptr) {
	    type = ogf_meta<double>::type();
	}
	AttributesManager& attrmgr = voxel_grob()->attributes();
	if(attrmgr.is_defined(attribute_name)) {
	    Logger::err("VoxelGrobEditor")
		<< attribute_name << "already defined." << std::endl;
	    return nullptr;
	}
	AttributeStore* store =
	    AttributeStore::create_attribute_store_by_element_type_name(
		type->name(),dimension
	    );
	if(store == nullptr) {
	    Logger::err("Attribute")
		<< "Could not create an attribute of type "
		<< type->name()
		<< std::endl;
	    return nullptr;
	}
	attrmgr.bind_attribute_store(attribute_name,store);
	return find_attribute(attribute_name);
    }

    NL::Vector* VoxelGrobEditor::find_or_create_attribute(
	const std::string& attribute_name, index_t dimension, MetaType* type
    ) {
	NL::Vector* result = find_attribute(attribute_name, true);
	
	if(result == nullptr) {
	    return create_attribute(attribute_name, dimension, type);
	}
	
	if(type != nullptr && result->get_element_meta_type() != type) {
	    Logger::err("VoxelEditor")
		<< "Attribute " << attribute_name
		<< " already exist with different element type"
		<< std::endl;
	    // This decrements the refcount that reaches 0
	    // thus deallocates the object.
	    result->unref();
	    return nullptr;
	}
	
	if(result->get_dimension() != dimension) {
	    Logger::err("VoxelEditor")
		<< "Attribute " << attribute_name
		<< " already exist with different dimension"
		<< std::endl;
	    // This decrements the refcount that reaches 0
	    // thus deallocates the object.	    
	    result->unref();
	    return nullptr;
	}

	return result;
    }
    
    bool VoxelGrobEditor::has_attribute(
        const std::string& attribute_name
    ) const {
	AttributesManager& attrmgr = voxel_grob()->attributes();
	return attrmgr.is_defined(attribute_name);
    }
}

