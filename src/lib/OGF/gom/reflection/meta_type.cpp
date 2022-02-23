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

#include <OGF/gom/reflection/meta_type.h>
#include <OGF/gom/reflection/meta_class.h>

//___________________________________________________

namespace OGF {

    MetaInformation::~MetaInformation() {
    }

    size_t MetaInformation::nb_custom_attributes() const {
	return CustomAttributes::nb_custom_attributes();
    }
    
    bool MetaInformation::has_custom_attribute(const std::string& name) const {
	return CustomAttributes::has_custom_attribute(name);
    }
    
    std::string MetaInformation::ith_custom_attribute_name(index_t i) const {
	return CustomAttributes::ith_custom_attribute_name(i);
    }
    
    std::string MetaInformation::ith_custom_attribute_value(index_t i) const {
	return CustomAttributes::ith_custom_attribute_value(i);
    }

    std::string MetaInformation::custom_attribute_value(const std::string& name) const {
	return CustomAttributes::custom_attribute_value(name);
    }
    
    /**************************************************************************/
    
    MetaType::MetaType(const std::string& name) : name_(name) {
    }
    
    MetaType::~MetaType(){
    }
    
    bool MetaType::is_a(const MetaType* other) const {
        return this == other ;
    }

    void MetaType::pre_delete() {
	clear_custom_attributes();
    }

}

