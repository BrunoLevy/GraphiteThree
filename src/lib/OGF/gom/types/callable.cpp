/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
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

#include <OGF/gom/types/callable.h>
#include <OGF/gom/reflection/meta.h>

namespace OGF {

    /*******************************************************************/

    Callable::Callable() : Object(true) {
    }
    
    Callable::~Callable() {
    }

    /*******************************************************************/

    Request::Request(Object* o, MetaMethod* m, bool managed) :
	object_(o),
	method_(m),
	managed_(managed)
    {
	if(managed_ && object_ != nullptr) {
	    object_->ref();
	}
    }
    
    Request::~Request() {
	if(managed_ && object_ != nullptr) {
	    object_->unref();
	}
    }

    bool Request::invoke(const ArgList& args, Any& ret_val) {
	return object_->invoke_method(method_->name(), args, ret_val);	
    }

    std::string Request::get_doc() const {
	std::string result;
	MetaMethod* mmethod = method();
	MetaClass* mclass = mmethod->container_meta_class();

	result  = "GOM function\n";
	result += "============\n";
	result += mclass->name() + "::" + mmethod->name() + "(";
	for(index_t i=0; i<mmethod->nb_args(); ++i) {
	    result += mmethod->ith_arg_name(i);
	    if(i != mmethod->nb_args()-1) {
		result += ",";
	    }
	}
	result += ")\n";
	if(mmethod->has_custom_attribute("help")) {
	    result += mmethod->custom_attribute_value("help");
	    result += "\n";
	}
	if(mmethod->nb_args() != 0) {
	    result += "Parameters\n";
	    result += "==========\n";
	    for(index_t i=0; i<mmethod->nb_args(); ++i) {
		result += mmethod->ith_arg_name(i);
		result += " : ";
		result += mmethod->ith_arg_type(i)->name();
		if(mmethod->ith_arg(i)->has_default_value()) {
		    result += " = ";
		    bool is_string = (
			mmethod->ith_arg(i)->default_value().meta_type() ==
			ogf_meta<std::string>::type()
		    );
		    if(is_string) {
			result += '\'';
		    }
		    result +=
			mmethod->ith_arg(i)->default_value().as_string();
		    if(is_string) {
			result += '\'';
		    }
		}
		result += "\n" ;
		if(mmethod->ith_arg(i)->has_custom_attribute("help")) {
		    result += mmethod->ith_arg(i)
			->custom_attribute_value("help");
		    result += "\n";
		}
	    }
	}

	return result;
    }
    
    /*******************************************************************/    
}
