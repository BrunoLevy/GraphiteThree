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

#include <OGF/gom/reflection/meta_property.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/types/gom.h>

//___________________________________________________

namespace OGF {

    MetaMethodSetProperty::MetaMethodSetProperty(
        const std::string& name, MetaProperty* property
    ) : MetaMethod(
        name, property->container_meta_class(), ogf_meta<void>::type()
    ),
        property_(property) {
        add_arg(MetaArg("value", property->type_name())) ;
    }
    
    MetaMethodSetProperty::~MetaMethodSetProperty() {
    }
    
    bool MetaMethodSetProperty::invoke(
        Object* target, const ArgList& args, Any& return_value
    ) {
        return MetaMethod::invoke(target, args, return_value) ;
    }

    MetaMethodGetProperty::MetaMethodGetProperty(
        const std::string& name, MetaProperty* property
    ) : MetaMethod(
        name, property->container_meta_class(), property->type_name()
    ),
        property_(property) {
    }

    MetaMethodGetProperty::~MetaMethodGetProperty() {
    }
    
    bool MetaMethodGetProperty::invoke(
        Object* target, const ArgList& args, Any& return_value
    ) {
        return MetaMethod::invoke(target, args, return_value) ;
    }


//___________________________________________________

    MetaProperty::MetaProperty(
        const std::string& name, MetaClass* container,
        const std::string& type_name, bool read_only
    ) : MetaMember(name,container), type_name_(type_name),
        read_only_(read_only) 
    {
        container->add_member(this) ;
        meta_method_get_ = new MetaMethodGetProperty(
            "get_"+name, this
        ) ;
        if(!read_only) {
            meta_method_set_ = new MetaMethodSetProperty(
                "set_"+name, this
            ) ;
        }
    }

    MetaProperty::MetaProperty(
        const std::string& name, MetaClass* container,
        MetaType* meta_type, bool read_only
    ) : MetaMember(name,container), type_name_(meta_type->name()),
        read_only_(read_only) {
        container->add_member(this) ;
        meta_method_get_ = new MetaMethodGetProperty(
            "get_"+name, this
        ) ;
        if(!read_only) {
            meta_method_set_ = new MetaMethodSetProperty(
                "set_"+name, this
            ) ;
        }
    }
    
    MetaProperty::~MetaProperty(){
    }

    bool MetaProperty::set_value(Object* target, const Any& value) {
        if(read_only_) {
            Logger::warn("MetaProperty") 
                << "MetaProperty "
                << container_meta_class()->name()
                << "::"
                << name()
                << " is read only"
                << std::endl ;
            return false ;
        }
        ArgList args ;
        args.create_arg("value", value) ;
        Any retval ;
        return meta_method_set_->invoke(target, args, retval) ;
    }
    
    bool MetaProperty::get_value(const Object* target, Any& value) {
        ArgList args ;
        return meta_method_get_->invoke(
            const_cast<Object*>(target), args, value
        );
    }

    bool MetaProperty::set_value(Object* target, const std::string& value) {
	Any value_any;
	value_any.set_value(value);
	return set_value(target, value_any);
    }
    
    bool MetaProperty::get_value(const Object* target, std::string& value) {
	Any value_any;
	bool ok = get_value(target, value_any);
	ok = ok && value_any.get_value(value);
	return ok;
    }

    MetaType* MetaProperty::type() const {
        return Meta::instance()->resolve_meta_type(type_name_) ;
    }
    

    void MetaProperty::set_read_only(bool x) { 
        read_only_ = x ;
        if(!read_only_ && meta_method_set_ == nullptr) {
            meta_method_set_ = new MetaMethodSetProperty(
                "set_" + name(), this
            ) ;
        }
    }

}



