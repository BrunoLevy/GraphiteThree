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
 
#include <OGF/devel/common/common.h>
#include <OGF/devel/types/properties.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/devel/commands/scene_graph_devel_commands.h>
// [includes insertion point] (do not delete this line)

namespace OGF {
    void devel_libinit::initialize() {

        Logger::out("Init") << "<devel>" << std::endl;

        //_____________________________________________________________

        gom_package_initialize(devel) ;
        ogf_register_grob_commands<SceneGraph,SceneGraphDevelCommands>();
        // [source insertion point] (do not delete this line)

        //_____________________________________________________________

        Module* module_info = new Module;
        module_info->set_name("devel");
        module_info->set_vendor("OGF");
        module_info->set_version("3-1.x");
        module_info->set_is_system(true);                        
        module_info->set_info("Graphite development tools");
        Module::bind_module("devel", module_info);

        Logger::out("Init") << "</devel>" << std::endl;        
    }
    
    void devel_libinit::terminate() {
        Logger::out("Init") << "<~devel>" << std::endl;                

        //_____________________________________________________________


        //_____________________________________________________________

        Module::unbind_module("devel") ;

        Logger::out("Init") << "</~devel>" << std::endl;                
    }

// You should not need to modify this file below that point.
    
/****************************************************************/


    devel_libinit::devel_libinit() {
        increment_users();
    }

    devel_libinit::~devel_libinit() {
        decrement_users();
    }
    
    void devel_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which 
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }
    
    void devel_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }
    
    int devel_libinit::count_ = 0;
}

// The initialization and termination functions
// are also declared using C linkage in order to 
// enable dynamic linking of modules.

extern "C" void DEVEL_API OGF_devel_initialize(void);
extern "C" void DEVEL_API OGF_devel_initialize() {
    OGF::devel_libinit::increment_users();
}

extern "C" void DEVEL_API OGF_devel_terminate(void);
extern "C" void DEVEL_API OGF_devel_terminate() {
    OGF::devel_libinit::decrement_users();
}




