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

#include <OGF/gom/types/connection.h>
#include <OGF/gom/types/object.h>

#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_type.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta_method.h>

namespace OGF {

/**************************************************************************/

    Connection::Connection(Object* source, const std::string& sig_name)	:
	source_(source),
	signal_name_(sig_name)
    {
        if(source_ != nullptr) {
            source_->add_connection(this) ;
        }
    }

    Connection::~Connection() {
    }

    bool Connection::test_arg_condition(
        const std::string& value, const std::string& condition
    ) const {
        if(condition.length() >= 2) {
            std::string oprel = condition.substr(0,2) ;
            unsigned int start = 2 ;
            while(start < condition.length() && condition[start] == ' ') {
                start++ ;
            }
            std::string rhs = condition.substr(
                start, condition.length() - start
            ) ;
            if(oprel == "==") {
                return value == rhs ;
            } else if(oprel == "!=") {
                return value != rhs ;
            }
        }
        return (value == condition) ;
    }

    bool Connection::test_arg_conditions(const ArgList& args_in) const {
        for(unsigned int i=0; i<conditions_.nb_args(); i++) {
            std::string arg_name = conditions_.ith_arg_name(i);
            std::string arg_condition =
		conditions_.ith_arg_value(i).as_string();
            if(!args_in.has_arg(arg_name)) {
                Logger::err("Connection") 
                    << "Condition on unexisting arg: "
                    << arg_name
                    << "==" << arg_condition
                    << std::endl ;
            } else {
                if(!test_arg_condition(
                       args_in.get_arg(arg_name), arg_condition
                   )
                ){
                    return false ;
                }
            }
        }
        return true ;
    }

    void Connection::translate_args(const ArgList& args_in, ArgList& args_out) {
        args_out.clear() ;
        for(unsigned int i=0; i<args_in.nb_args(); i++) {
            if(discard_args_.find(
                   args_in.ith_arg_name(i)) != discard_args_.end()
            ) {
                continue ;
            }
            if(rename_args_.has_arg(args_in.ith_arg_name(i))) {
                args_out.create_arg(
                    rename_args_.get_arg(args_in.ith_arg_name(i)),
                    args_in.ith_arg_value(i)
                );
            } else {
                args_out.create_arg(
                    args_in.ith_arg_name(i),
                    args_in.ith_arg_value(i)
                ) ;
            }
        }
        args_out.append(args_) ;
    }

    bool Connection::invoke(const ArgList& args_in, Any& ret_val) {
        if(!test_arg_conditions(args_in)) {
            return true ;
        }
        ArgList args ;
        translate_args(args_in, args) ;
	return invoke_target(args, ret_val);
    }
    
/******************************************************************************/

    SlotConnection::SlotConnection(
	Object* source, const std::string& sig_name,
	Object* target, const std::string& slot_name
    ) :
	Connection(source, sig_name),
	target_(target),
	slot_name_(slot_name)
    {
    }
    
    bool SlotConnection::invoke_target(
	const ArgList& args, Any& ret_val
    ) {
	return target_->invoke_method(slot_name_, args, ret_val);
    }
    
/******************************************************************************/

    CallableConnection::CallableConnection(
	Object* source, const std::string& sig_name, Callable* target
    ) :
	Connection(source, sig_name),
	target_(target)
    {
    }
    
    bool CallableConnection::invoke_target(
	const ArgList& args, Any& ret_val
    ) {
	return target_->invoke(args, ret_val);
    }

/******************************************************************************/
    
}
