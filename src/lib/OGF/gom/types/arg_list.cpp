/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
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

#include <OGF/gom/types/arg_list.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_type.h>

namespace OGF {

    index_t ArgList::find_arg_index(const std::string& name) const {
        for(index_t i=0; i<nb_args(); ++i) {
            if(argname_[i] == name) {
                return i;
            }
        }
        return index_t(-1);
    }

    void ArgList::append(const ArgList& rhs, bool overwrite) {
        for(index_t i=0; i<rhs.nb_args(); ++i) {
            append_ith_arg(rhs, i, overwrite);
        }
    }

    void ArgList::append_ith_arg(
        const ArgList& rhs, index_t i, bool overwrite
    ) {
        geo_debug_assert(i < rhs.nb_args());
        index_t j = find_arg_index(rhs.argname_[i]);
        if(j == index_t(-1)) {
	    argval_.push_back(rhs.argval_[i]);
	    argname_.push_back(rhs.argname_[i]);
        } else {
            if(overwrite) {
		argval_[j] = rhs.argval_[i];
            }
        }
    }

    void ArgList::serialize(std::ostream& out) const {
        out << '(';
        for(index_t i=0; i<nb_args(); ++i) {
            out << ith_arg_name(i) << ":" << ith_arg_value(i).as_string() << " ";
        }
        out << ')';
    }

    void ArgList::arg_type_error(
	index_t i, const std::string& expected_typeid_name
    ) const {
	MetaType* expected_type = Meta::instance()->
	    resolve_meta_type_by_typeid_name(expected_typeid_name);

	MetaType* current_type = argval_[i].meta_type();

	Logger::err("GOM")
            << "Arg type error:"
            << " i = " << i
            << " name = " << argname_[i]
            << " type = " <<(
	    argval_[i].is_null() ? "null" : (
		((current_type != nullptr) ? current_type->name() : "unknown")
	    ))
            << " expected type = "
            << ((expected_type != nullptr) ? expected_type->name() :
                "unknown: " + expected_typeid_name)
            << std::endl;

	std::ostringstream oss;
	serialize(oss);
	oss << std::ends;
	Logger::err("GOM") << "Arglist = " << oss.str() << std::endl;
	//geo_assert_not_reached;
    }

    bool ArgList::get_object_name(const Any& object, Any& name) {
        Object* object_ptr;
        if(!object.get_value(object_ptr)) {
            return false;
        }
        if(object_ptr == nullptr) {
            return false;
        }
        std::string object_name;
        if(!object_ptr->get_property("name", object_name)) {
            return false;
        }
        name.set_value(object_name);
        return true;
    }

}
