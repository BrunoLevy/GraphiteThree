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

#include <OGF/gom/reflection/meta_method.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta_arg.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/types/gom.h>

//___________________________________________________

namespace OGF {

    MetaMethod::MetaMethod(
        const std::string& name,
        MetaClass* container,
        const std::string& return_type_name 
    ) : MetaMember(name,container),
        return_type_name_(return_type_name), 
        adapter_(nullptr) {
    }
    
    MetaMethod::MetaMethod(
        const std::string& name,
        MetaClass* container,
        MetaType* return_type
    ) : MetaMember(name,container),
        return_type_name_(return_type->name()), 
        adapter_(nullptr) {
    }

    MetaMethod::~MetaMethod(){
    }

    void MetaMethod::pre_delete() {
	MetaMember::pre_delete();
	meta_args_.clear();
    }


    std::string MetaMethod::get_doc() const {
	std::string result;
	MetaClass* mclass = container_meta_class();

	result  = "GOM function\n";
	result += "============\n";
	result += mclass->name() + "::" + name() + "(";
	for(index_t i=0; i<nb_args(); ++i) {
	    result += ith_arg_name(i);
	    if(i != nb_args()-1) {
		result += ",";
	    }
	}
	result += ")\n";
	if(has_custom_attribute("help")) {
	    result += custom_attribute_value("help");
	    result += "\n";
	}
	if(nb_args() != 0) {
	    result += "Parameters\n";
	    result += "==========\n";
	    for(index_t i=0; i<nb_args(); ++i) {
		result += ith_arg_name(i);
		result += " : ";
		result += ith_arg_type(i)->name();
		if(ith_arg(i)->has_default_value()) {
		    result += " = ";
		    bool is_string = (
			ith_arg(i)->default_value().meta_type() ==
			ogf_meta<std::string>::type()
		    );
		    if(is_string) {
			result += '\'';
		    }
		    result +=
			ith_arg(i)->default_value().as_string();
		    if(is_string) {
			result += '\'';
		    }
		}
		result += "\n" ;
		if(ith_arg(i)->has_custom_attribute("help")) {
		    result += ith_arg(i)
			->custom_attribute_value("help");
		    result += "\n";
		}
	    }
	}
	return result;
    }
    
    
    bool MetaMethod::has_arg(const std::string& meta_arg_name) {
        return (find_arg(meta_arg_name) != nullptr) ;
    }
    
    const MetaArg* MetaMethod::find_arg(
        const std::string& meta_arg_name
    ) const {
        for(index_t i=0; i<nb_args(); i++) {
            if(ith_arg(i)->name() == meta_arg_name) {
                return ith_arg(i) ;
            }
        }
        return nullptr ;
    }

    MetaArg* MetaMethod::find_arg(const std::string& meta_arg_name) {
        for(index_t i=0; i<nb_args(); i++) {
            if(ith_arg(i)->name() == meta_arg_name) {
                return ith_arg(i) ;
            }
        }
        return nullptr ;
    }

    bool MetaMethod::invoke(
        Object* target, const ArgList& args, Any& return_value
    ) {
        if(method_adapter() == nullptr) {
            Logger::err("MetaMethod") << "MetaMethod "
                                      << container_meta_class()->name()
                                      << "::"
                                      << name()
                                      << " does not have a method adapter"
                                      << std::endl ;
            return false ;
        }
        if(!check_args(args)) {
            Logger::err("MetaMethod") << "MetaMethod "
                                      << container_meta_class()->name()
                                      << "::"
                                      << name()
                                      << " : missing arguments"
                                      << std::endl ;
            Logger::err("MetaMethod") << " got:   " << std::endl ;
            for(unsigned int i=0; i<args.nb_args(); i++) {
                Logger::err("MetaMethod") << "    "
                                          << args.ith_arg_name(i)
                                          << " = "
                                          << args.ith_arg_value(i).as_string()
                                          << std::endl ;
            }
            return false ;
        }
        if(nb_default_args(args) != 0) {
            ArgList all_args = args ;
            add_default_args(all_args) ;
            return method_adapter()(target, name(), all_args, return_value) ;
        }
        return method_adapter()(target, name(), args, return_value) ;
    }

    bool MetaMethod::emit_signal(
        Object* target, const std::string& sig_name, 
        const ArgList& args, bool called_from_slot
    ) {
        return target->emit_signal(sig_name, args, called_from_slot) ;
    }

    MetaType* MetaMethod::return_type() const {
        return Meta::instance()->resolve_meta_type(return_type_name_) ;
    }

    bool MetaMethod::check_args(const ArgList& args) {
	// Special case: method declared to take an ArgList as
	// argument.
	if(nb_args() == 1 && ith_arg_type_name(0) == "OGF::ArgList") {
	    return true;
	}
	// Regular case: check all arguments.
        for(index_t i=0; i<nb_args(); i++) {
            const MetaArg* arg = ith_arg(i) ;
            bool arg_ok = (
                arg->has_default_value() || args.has_arg(arg->name())
            ) ;
            if(!arg_ok) {
                return false ;
            }
        }
        return true ;
    }

    index_t MetaMethod::nb_used_args(const ArgList& args) {
        index_t result = 0 ;
        for(index_t i=0; i<args.nb_args(); i++) {
            if(has_arg(args.ith_arg_name(i))) {
                result++ ;
            } 
        }
        return result ;
    }

    index_t MetaMethod::nb_default_args(const ArgList& args) {
	// Special case: method declared as taking an ArgList.
	if(nb_args() == 1 && ith_arg_type_name(0) == "OGF::ArgList") {
	    return 0;
	}
	// Regular case: check for required number of default args.
        index_t result = 0 ;
        for(index_t i=0; i<nb_args(); i++) {
            const MetaArg* arg = ith_arg(i) ;
            if(!args.has_arg(arg->name())) {
                result++ ;
            }
        }
        return result ;
    }

    void MetaMethod::add_default_args(ArgList& args) {
        for(index_t i=0; i<nb_args(); i++) {
            const MetaArg* arg = ith_arg(i) ;
            if(!args.has_arg(arg->name())) {
                ogf_assert(arg->has_default_value()) ;
                args.create_arg(arg->name(), arg->default_value()) ;
            }
        }
    }

    bool MetaMethod::ith_arg_has_custom_attribute(
	index_t i, const std::string& name
    ) const {
	return ith_arg(i)->has_custom_attribute(name);
    }

    std::string MetaMethod::ith_arg_custom_attribute_value(
	index_t i, const std::string& name
    ) const {
	return ith_arg(i)->custom_attribute_value(name);
    }

    size_t MetaMethod::ith_arg_nb_custom_attributes(index_t i) {
	return ith_arg(i)->nb_custom_attributes();
    }
    
    std::string MetaMethod::ith_arg_jth_custom_attribute_name(
	index_t i, index_t j
    ) {
	return ith_arg(i)->ith_custom_attribute_name(j);
    }
    
    std::string MetaMethod::ith_arg_jth_custom_attribute_value(
	index_t i, index_t j
    ) {
	return ith_arg(i)->ith_custom_attribute_value(j);
    }
}



