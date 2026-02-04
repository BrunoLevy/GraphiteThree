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


#include <OGF/scene_graph/commands/commands.h>
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_class.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/command_line.h>
#include <sstream>

namespace OGF {

/**************************************************/

    Interface::Interface() : grob_(nullptr) {
    }

    Interface::~Interface() {
    }

    void Interface::set_grob(Grob* grob) {
        grob_ = grob ;
    }

    SceneGraph* Interface::scene_graph() const {
        return grob_->scene_graph() ;
    }


/**************************************************/

    bool Commands::command_is_running_ = false;

    Commands::Commands() : chrono_(true) {
    }

    Commands::~Commands() {
    }

    bool Commands::invoke_method(
        const std::string& method_name,
        const ArgList& args_in, Any& ret_val
    ) {

        if(!args_in.has_arg("override_lock") && command_is_running_) {
	    // Do not invoke command while another command is running
	    MetaMethod* mmethod = meta_class()->find_method(method_name);
	    if( // ... but this does not concern member function of base classes
		mmethod->container_meta_class()->is_subclass_of(
		    ogf_meta<OGF::Commands>::meta_class()
		)
	    ) {
		Logger::warn("Commands")
		    << "Tryed to invoke command from locked Commands class"
		    << std::endl ;
		return false ;
	    }
        }

        command_is_running_ = true ;

        if(get_grob() == nullptr) {
            command_is_running_ = false ;
            return true ;
        }


        bool invoked_from_gui = false;

        // Copy argument list, ignore arguments that start with '_'
        ArgList args;
        for(index_t i=0; i<args_in.nb_args(); ++i) {
            const std::string& name = args_in.ith_arg_name(i);
            const Any& value = args_in.ith_arg_value(i);
            if(name.length() > 0 && name[0] == '_') {
                if(name == "_invoked_from_gui" && value.as_string() == "true") {
                    invoked_from_gui = true;
                }
                continue;
            }
            args.create_arg(name, value);
        }


        if(interpreter() != nullptr) {
            if(invoked_from_gui) {
                Object* main = interpreter()->resolve_object("main");
                main->invoke_method("save_state");
		interpreter()->record_invoke_in_history(this, method_name, args);
            }
        }

        // Do not display timings for methods with continuous updates
        // (e.g. set_multiresolution_level)
        MetaMethod* mmethod = meta_class()->find_method(method_name) ;
        bool do_timings = chrono_ && mmethod != nullptr &&
            !mmethod->has_custom_attribute("continuous_update") ;

        if(get_grob() != nullptr) {
            if(do_timings) {

		// TODO: prevent user from changing current object
                // during execution the command.

                std::string full_name =
                    meta_class()->name() + "::" + method_name ;
                Stopwatch timer ;
                Logger::out("timings")
                    << "(" << full_name << ") starting ..." << std::endl ;
                bool result = Interface::invoke_method(
                    method_name, args, ret_val
                ) ;
                Logger::out("timings")
                    << "(" << full_name << ") Elapsed time: "
                    << timer.elapsed_time() << std::endl ;

		// TODO: re-enable changing current object here.

		//   If the user clicked on the object list attempting
		// to change current object, restore selected item.

		// TODO: take into account user changed object here

		command_is_running_ = false ;
                return result ;
            } else {
                command_is_running_ = false ;
                return Interface::invoke_method(method_name, args, ret_val) ;
            }
        }

        command_is_running_ = false ;
        return true ;
    }

    Interpreter* Commands::interpreter() {
	if(get_grob() == nullptr) {
	    return nullptr;
	}
	return get_grob()->interpreter();
    }

}
