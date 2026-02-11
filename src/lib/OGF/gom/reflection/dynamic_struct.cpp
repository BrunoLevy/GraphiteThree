/*
 *  GXML/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
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
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 */

#include <OGF/gom/reflection/dynamic_struct.h>

namespace OGF {

    MetaStruct::MetaStruct(
	const std::string& struct_name
    ) :  MetaClass(struct_name, "OGF::Object") {
    }

    MetaStruct::~MetaStruct() {
    }

    MetaProperty* MetaStruct::add_property(
	const std::string& property_name,
	MetaType* property_type
    ) {
	MetaProperty* result = new MetaProperty(
	    property_name, this, property_type
	);
	return result;
    }

    MetaProperty* MetaStruct::add_property(
	const std::string& property_name,
	const std::string& property_type_name
    ) {
	MetaProperty* result = new MetaProperty(
	    property_name, this, property_type_name
	);
	return result;
    }

    /****************************************************************/

    MetaBuiltinStruct::MetaBuiltinStruct(
	const std::string& struct_name
    ) :  MetaBuiltinType(struct_name) {
	meta_struct_ = new MetaStruct(
	    struct_name + "_as_gom_Object"
	);
    }

    MetaBuiltinStruct::~MetaBuiltinStruct() {
    }

    MetaProperty* MetaBuiltinStruct::add_property(
	const std::string& property_name,
	MetaType* property_type
    ) {
	return meta_struct_->add_property(property_name, property_type);
    }

    MetaProperty* MetaBuiltinStruct::add_property(
	const std::string& property_name,
	const std::string& property_type_name
    ) {
	return meta_struct_->add_property(property_name, property_type_name);
    }

    /****************************************************************/

}
