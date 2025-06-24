/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 Bruno Levy
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

#include <OGF/basic/common/common.h>
#include <OGF/basic/modules/module.h>
#include <OGF/basic/modules/modmgr.h>
#include <OGF/basic/os/file_manager.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/process.h>

namespace OGF {

/****************************************************************/

    void basic_libinit::initialize() {

        // Do not install geogram signal handlers to
        // facilitate debugging under Windows.
        GEO::initialize(
            GEOGRAM_INSTALL_LOCALE |
            GEOGRAM_INSTALL_FPE    |
            GEOGRAM_INSTALL_BIBLIO
        );
        CmdLine::import_arg_group("standard");
        CmdLine::import_arg_group("gfx");
	CmdLine::set_arg("gfx:full_screen", true);

	if(ModuleManager::resolve_symbol("graphite_main") == nullptr) {
	    CmdLine::set_arg(
		"log:features_exclude",
		"Init;FileManager;timings"
	    );
	}

        //   Whenever an assertion fails, abort the
        // program.
        CmdLine::set_arg("sys:assert","abort");
        Logger::out("Init") << "<basic>" << std::endl;

        //_____________________________________________________________

        FileManager::initialize();
        ModuleManager::initialize();

        // Graphite major version number is 3.
        // Graphite minor and release is geogram version.
        std::string geogram_version =
	    Environment::instance()->get_value("version");
        Environment::instance()->set_value("version", "3-" + geogram_version);
        Environment::instance()->set_value(
            "nb_cores", String::to_string(Process::number_of_cores())
        );

        std::string geogram_SVN_revision=
	    Environment::instance()->get_value("SVN revision");

        //_____________________________________________________________

        Module* module_info = new Module;
        module_info->set_name("basic");
        module_info->set_vendor("OGF");
        module_info->set_version("3-1.x");
        module_info->set_info("Basic types, services, OS abstraction layer");
        module_info->set_is_system(true);
        Module::bind_module("basic", module_info);

        Logger::out("Init") << "</basic>" << std::endl;
    }

    void basic_libinit::terminate() {
        Logger::out("Init") << "<~basic>" << std::endl;

        //_____________________________________________________________


        ModuleManager::terminate_dynamic_modules();
        Module::unbind_module("basic");
        ModuleManager::terminate();
        FileManager::terminate();

        //_____________________________________________________________

        Logger::out("Init") << "</~basic>" << std::endl;
        // Note: Geogram is automatically terminated, using atexit()
    }

// You should not need to modify this file below that point.

/****************************************************************/

    basic_libinit::basic_libinit() {
        increment_users();
    }

    basic_libinit::~basic_libinit() {
        decrement_users();
    }

    void basic_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }

    void basic_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }

    int basic_libinit::count_ = 0;

}

// The initialization and termination functions
// are also declared using C linkage in order to
// enable dynamic linking of modules.

extern "C" void BASIC_API OGF_basic_initialize(void);
extern "C" void BASIC_API OGF_basic_initialize() {
    OGF::basic_libinit::increment_users();
}

extern "C" void BASIC_API OGF_basic_terminate(void);
extern "C" void BASIC_API OGF_basic_terminate() {
    OGF::basic_libinit::decrement_users();
}
