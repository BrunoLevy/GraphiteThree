
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
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
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Bruno Levy - Bruno.Levy@inria.fr
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
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */


#include <OGF/gompy/common/common.h>
#include <OGF/gompy/interpreter/python_interpreter.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>

// [includes insertion point] (do not delete this line)

extern "C" {
    extern int Py_IsInitialized(void);
}

namespace OGF {

    void gom_python_libinit::initialize() {
        Logger::out("Init") << "<gom_python>" << std::endl;

        /*************************************************************/

        gom_package_initialize(gompy) ;

        // [source insertion point] (do not delete this line)

        // Insert package initialization stuff here ...
	if(!Py_IsInitialized()) {
	    Interpreter* interp = new PythonInterpreter();
	    Interpreter::initialize(interp, "Python", "py");
	}

        /*************************************************************/

        Module* module_info = new Module;
        module_info->set_name("gom_python");
        module_info->set_vendor("Bruno Levy - Bruno.Levy@inria.fr");
        module_info->set_version("3-1.x");
        module_info->set_info("Binding for the Python language");
        Module::bind_module("gom_python", module_info);
        Logger::out("Init") << "</gom_python>" << std::endl;
    }

    void gom_python_libinit::terminate() {
        Logger::out("Init") << "<~gom_python>" << std::endl;

        /*************************************************************/

	Interpreter::terminate("Python", "py");

        /*************************************************************/

        Module::unbind_module("gom_python");
        Logger::out("Init") << "</~gom_python>" << std::endl;
    }

    gom_python_libinit::gom_python_libinit() {
        increment_users();
    }

    gom_python_libinit::~gom_python_libinit() {
        decrement_users();
    }

    void gom_python_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }

    void gom_python_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }

    int gom_python_libinit::count_ = 0;
}

// The initialization and termination functions
// are also declared using C linkage in order to
// enable dynamic linking of modules.

extern "C" void gompy_API OGF_gompy_initialize(void);
extern "C" void gompy_API OGF_gompy_initialize() {
    OGF::gom_python_libinit::increment_users();
}

extern "C" void gompy_API OGF_gompy_terminate(void);
extern "C" void gompy_API OGF_gompy_terminate() {
    OGF::gom_python_libinit::decrement_users();
}
