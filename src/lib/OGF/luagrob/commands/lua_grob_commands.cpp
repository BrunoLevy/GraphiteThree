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
 

#include <OGF/luagrob/commands/lua_grob_commands.h>

namespace OGF {
    
    LuaGrobCommands::LuaGrobCommands() { 
    }
        
    LuaGrobCommands::~LuaGrobCommands() { 
    }        

    /****************************************/

    LuaGrobProgramCommands::LuaGrobProgramCommands() {
    }

    LuaGrobProgramCommands::~LuaGrobProgramCommands() {
    }

    void LuaGrobProgramCommands::run() {
	lua_grob()->execute_program();
    }

    void LuaGrobProgramCommands::set_autorun(bool autorun) {
	lua_grob()->set_autorun(autorun);
    }
    
    void LuaGrobProgramCommands::load_program(const LuaFileName& value) {
	lua_grob()->load_program_source(value);
    }

    void LuaGrobProgramCommands::save_program(const NewLuaFileName& value) {
	lua_grob()->save_program_source(value);	
    }

    void LuaGrobProgramCommands::clear() {
	lua_grob()->set_source("");
    }
    
    /****************************************/
    
    LuaGrobShaderCommands::LuaGrobShaderCommands() {
    }

    LuaGrobShaderCommands::~LuaGrobShaderCommands() {
    }
    
    void LuaGrobShaderCommands::load_shader(const LuaFileName& value) {
	lua_grob()->load_shader_source(value);
    }

    void LuaGrobShaderCommands::save_shader(const NewLuaFileName& value) {
	lua_grob()->save_shader_source(value);	
    }

    void LuaGrobShaderCommands::clear() {
	lua_grob()->set_shader_source("function draw()\nend");
    }

}

