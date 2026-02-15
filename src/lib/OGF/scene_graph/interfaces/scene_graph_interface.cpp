/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
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
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 *
 * As an exception to the GPL, Graphite can be linked with the following
 *  (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#include <OGF/scene_graph/interfaces/scene_graph_interface.h>

namespace OGF {

    SceneGraphInterface::SceneGraphInterface() {
    }

    SceneGraphInterface::~SceneGraphInterface() {
    }

    bool SceneGraphInterface::invoke_method(
	const std::string& method_name, const ArgList& args_in, Any& ret_val
    ) {

	bool invoked_from_gui = false;

        // Copy argument list, ignore arguments that start with '_'
        ArgList args;
        for(index_t i=0; i<args_in.nb_args(); ++i) {
            const std::string& name = args_in.ith_arg_name(i);
            const Any& value = args_in.ith_arg_value(i);
            if(name.length() > 0 && name[0] == '_') {
                if(name == "_invoked_from_gui" &&
		   value.as_string() == "true") {
                    invoked_from_gui = true;
                }
                continue;
            }
            args.create_arg(name, value);
        }

        if(scene_graph() != nullptr && invoked_from_gui) {
	    Interpreter* interpreter = scene_graph()->interpreter();
	    if(interpreter != nullptr) {
                Object* main = interpreter->resolve_object(
		    "main"
		);
		if(main != nullptr) {
		    main->invoke_method("save_state");
		}
		interpreter->record_invoke_in_history(
		    this, method_name, args
		);
            }
        }

	return Interface::invoke_method(method_name, args, ret_val) ;
    }


}
