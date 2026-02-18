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


#include <OGF/skin_imgui/common/common.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/basic/modules/module.h>
#include <OGF/basic/os/file_manager.h>

#include <geogram_gfx/gui/application.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/environment.h>

#include <stdlib.h>


namespace OGF {

/****************************************************************/

    void skin_imgui_libinit::initialize() {
        Logger::out("Init") << "<skin_imgui>" << std::endl;

        /***************************************************************/

        gom_package_initialize(skin_imgui) ;
        Environment::instance()->set_value(
	    "gui_styles",
	    ::GEO::Application::get_styles()
	) ;

        /***************************************************************/

        Module* module_info = new Module ;
        module_info->set_name("skin_imgui") ;
        module_info->set_vendor("OGF") ;
        module_info->set_version("3-1.x") ;
        module_info->set_is_system(true);
        module_info->set_info("Implementation of Skin based on ImGUI") ;
        Module::bind_module("skin_imgui", module_info) ;

        Logger::out("Init") << "</skin_imgui>" << std::endl;
    }

    void skin_imgui_libinit::terminate() {
        Logger::out("Init") << "<~skin_imgui>" << std::endl;

        /***************************************************************/
        // Insert termination code for library skin_imgui here.

        /***************************************************************/

        Module::unbind_module("skin_imgui") ;

        Logger::out("Init") << "</~skin_imgui>" << std::endl;
    }

// You should not need to modify this file below that point.

/****************************************************************/

    skin_imgui_libinit::skin_imgui_libinit() {
        increment_users() ;
    }

    skin_imgui_libinit::~skin_imgui_libinit() {
        decrement_users() ;
    }

    void skin_imgui_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which
        // may cause duplicate initialization of libraries.
        count_++ ;
        if(count_ == 1) {
            initialize() ;
        }
    }

    void skin_imgui_libinit::decrement_users() {
        count_-- ;
        if(count_ == 0) {
            terminate() ;
        }
    }

    int skin_imgui_libinit::count_ = 0 ;

}

// The initialization and termination functions
// are also declared using C linkage in order to
// enable dynamic linking of modules.

extern "C" void SKIN_IMGUI_API OGF_skin_imgui_initialize(void);
extern "C" void SKIN_IMGUI_API OGF_skin_imgui_initialize() {
    OGF::skin_imgui_libinit::increment_users() ;
}

extern "C" void SKIN_IMGUI_API OGF_skin_imgui_terminate(void);
extern "C" void SKIN_IMGUI_API OGF_skin_imgui_terminate() {
    OGF::skin_imgui_libinit::decrement_users() ;
}
