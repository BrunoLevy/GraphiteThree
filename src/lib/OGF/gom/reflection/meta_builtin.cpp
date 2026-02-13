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

#include <OGF/gom/reflection/meta_builtin.h>
#include <OGF/gom/reflection/meta.h>

/*************************************************************************/

namespace OGF {

    MetaBuiltinType::MetaBuiltinType(
        const std::string& name
    ):MetaType(name) {
    }

    MetaBuiltinType::~MetaBuiltinType() {
    }

    bool MetaBuiltinType::is_pointer_type() const {
        return (
            name().length() >= 2 &&
            name()[name().length() - 1] == '*'
        ) ;
    }

    bool MetaBuiltinType::is_subtype_of(const MetaType* other) const {
	const MetaBuiltinType* other_as_builtin =
	    dynamic_cast<const MetaBuiltinType*>(other);
	if(
	    other_as_builtin != nullptr &&
	    is_pointer_type() && other_as_builtin->is_pointer_type()
	) {
	    std::string pointed_type = name().substr(0, name().length()-1);
	    std::string other_pointed_type = name().substr(0, name().length()-1);
	    MetaType* pointed_mtype = Meta::instance()->resolve_meta_type(
		pointed_type
	    );
	    MetaType* other_pointed_mtype = Meta::instance()->resolve_meta_type(
		other_pointed_type
	    );
	    if(
		pointed_mtype != nullptr &&
		other_pointed_mtype != nullptr &&
		pointed_mtype->is_subtype_of(other_pointed_mtype)
	    ) {
		return true;
	    }
	}
	return MetaType::is_subtype_of(other);
    }

}
