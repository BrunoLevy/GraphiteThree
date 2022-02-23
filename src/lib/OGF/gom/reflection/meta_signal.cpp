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

#include <OGF/gom/reflection/meta_signal.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/types/gom.h>

//___________________________________________________

namespace OGF {

    bool MetaSignal::signal_method_adapter(
        Object* target, 
        const std::string& method_name, const ArgList& args,
        Any& ret_val
    ) {
        bool result = emit_signal(target, method_name, args, true) ;
	ret_val.set_value(result);
        return true;
    }

    MetaSignal::MetaSignal(
        const std::string& name, MetaClass* container
    ) : MetaMethod(name, container, ogf_meta<void>::type()) {
        container->add_member(this) ;
        set_method_adapter(signal_method_adapter) ;
    }

    MetaSignal::~MetaSignal(){
    }

}



