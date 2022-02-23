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
 *  (non-GPL) libraries: Qt, SuperLU, WildMagic and CGAL
 */
 
#include <OGF/luagrob/common/common.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>

#include <OGF/luagrob/grob/lua_grob.h>
#include <OGF/luagrob/shaders/lua_grob_shader.h>
#include <OGF/luagrob/commands/lua_grob_commands.h>

#include <geogram/basic/command_line.h>

// [includes insertion point] (do not delete this line)

namespace OGF {
    void luagrob_libinit::initialize() {
        Logger::out("Init") << "<luagrob>" << std::endl; 
        //_____________________________________________________________
        gom_package_initialize(luagrob) ;
        
        ogf_register_grob_type<LuaGrob>();
        ogf_register_grob_shader<LuaGrob,PlainLuaGrobShader>();
        

        ogf_register_grob_commands<LuaGrob, LuaGrobProgramCommands>();
        ogf_register_grob_commands<LuaGrob, LuaGrobShaderCommands>();	
	
        // [source insertion point] (do not delete this line)
        // Insert package initialization stuff here ...
        //_____________________________________________________________
        Module* module_info = new Module;
        module_info->set_name("luagrob");
        module_info->set_vendor("OGF");
        module_info->set_is_system(true);        
        module_info->set_version("3-1.x");
        module_info->set_info(
            "Luagrob grids object and manipulation"
        );
        Module::bind_module("luagrob", module_info);

        Logger::out("Init") << "</luagrob>" << std::endl;         
    }
    
    void luagrob_libinit::terminate() {
        Logger::out("Init") << "<~luagrob>" << std::endl;                 
        //_____________________________________________________________
        // Insert package termination stuff here ...
        
        //_____________________________________________________________

        Module::unbind_module("luagrob");
        Logger::out("Init") << "</~luagrob>" << std::endl;        
    }
    
    luagrob_libinit::luagrob_libinit() {
        increment_users();
    }
    luagrob_libinit::~luagrob_libinit() {
        decrement_users();
    }
    
    void luagrob_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which 
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }
    
    void luagrob_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }
    
    int luagrob_libinit::count_ = 0;
}
// The initialization and termination functions
// are also declared using C linkage in order to 
// enable dynamic linking of modules.
extern "C" void LUAGROB_API OGF_luagrob_initialize(void);
extern "C" void LUAGROB_API OGF_luagrob_initialize() {
    OGF::luagrob_libinit::increment_users();
}

extern "C" void LUAGROB_API OGF_luagrob_terminate(void);
extern "C" void LUAGROB_API OGF_luagrob_terminate() {
    OGF::luagrob_libinit::decrement_users();
}

