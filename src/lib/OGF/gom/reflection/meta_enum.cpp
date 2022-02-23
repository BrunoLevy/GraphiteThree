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

#include <OGF/gom/reflection/meta_enum.h>
#include <string>

//___________________________________________________

namespace OGF {

    MetaEnum::MetaEnum(
        const std::string& enum_name
    ) : MetaType(enum_name) {
        set_serializer(new EnumSerializer(this)) ;
    }

    MetaEnum::~MetaEnum(){
    }


    void MetaEnum::add_value(const std::string& name, int value) {
        ogf_assert(!has_value(name)) ;
        ogf_assert(!has_value(value)) ;
        Value val ;
        val.name = name ;
        val.value = value ;
        values_.push_back(val) ;
    }
    
    bool MetaEnum::has_value(int value) const {
        for(unsigned int i=0; i<values_.size(); i++) {
            if(values_[i].value == value) {
                return true ;
            }
        }
        return false ;
    }
    
    bool MetaEnum::has_value(const std::string& name) const {
        for(unsigned int i=0; i<values_.size(); i++) {
            if(values_[i].name == name) {
                return true ;
            }
        }
        return false ;
    }
    
    int MetaEnum::get_value_by_name(const std::string& name) const {
        for(unsigned int i=0; i<values_.size(); i++) {
            if(values_[i].name == name) {
                return values_[i].value ;
            }
        }
        bool found = false ;
        ogf_assert(found) ;
        return -1 ;
    }
    
    const std::string& MetaEnum::get_name_by_value(int value) const {
        for(unsigned int i=0; i<values_.size(); i++) {
            if(values_[i].value == value) {
                return values_[i].name ;
            }
        }
        bool found = false ;
        ogf_assert(found) ;
        static std::string empty;
        return empty;
    } 

}



