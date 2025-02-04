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

#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/gom/lua/interop.h>
#include <OGF/gom/lua/lua_graphite_object.h>
#include <OGF/gom/lua/vec_mat_interop.h>
#include <OGF/gom/types/connection.h>
#include <OGF/gom/types/node.h>
#include <OGF/gom/types/callable.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/basic/os/file_manager.h>
#include <OGF/basic/modules/modmgr.h>
#include <geogram/basic/process.h>
#include <geogram/lua/lua_io.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

#include <sstream>

/*************************************************************************/

namespace OGF {
    using namespace OGF::GOMLua;

    LuaInterpreter::LuaInterpreter() {
	lua_state_ = luaL_newstate();
	luaL_openlibs(lua_state_);
	init_lua_graphite(this);
	init_lua_io(lua_state_);
    }

    LuaInterpreter::~LuaInterpreter() {
	lua_close(lua_state_);
    }

    void LuaInterpreter::reset() {
	lua_close(lua_state_);
	lua_state_ = luaL_newstate();
	luaL_openlibs(lua_state_);
	init_lua_graphite(this);
	init_lua_io(lua_state_);
    }

    bool LuaInterpreter::execute(
        const std::string& command, bool save_in_history, bool log
    ) {
	bool result = true;

        if(log) {
            Logger::out("GOMLua") << command << std::endl;
        }

	if(luaL_dostring(lua_state_,command.c_str())) {
	    adjust_lua_state();
	    const char* msg = lua_tostring(lua_state_,-1);
	    display_error_message(msg);
	    result = false;
	}
        if(save_in_history) {
            add_to_history(command);
        }
        return result;
    }

    bool LuaInterpreter::execute_file(const std::string& file_name_in) {
	std::string file_name = file_name_in;
        Environment::instance()->set_value("current_gel_file", file_name);
        if(!FileManager::instance()->find_file(file_name)) {
            Logger::err("GOMLua") << "Cannot find file \'"
				  << file_name << "\'" << std::endl;
            return false;
        }
	bool result = true;
	if(luaL_dofile(lua_state_,file_name.c_str())) {
	    adjust_lua_state();
	    const char* msg = lua_tostring(lua_state_,-1);
	    Logger::err("Lua") << msg << std::endl;
	    result = false;
	}
        return result;
    }

    void LuaInterpreter::bind(const std::string& id, const Any& value) {
	lua_pushgraphiteval(lua_state_, value);
	lua_setglobal(lua_state_, id.c_str());
    }

    Any LuaInterpreter::resolve(const std::string& name, bool quiet) const {
	Any any_result;
	Object* result = Interpreter::resolve_object_by_global_id(name, true);
	if(result != nullptr) {
	    any_result.set_value(result);
	    return any_result;
	}

	lua_getglobal(lua_state_, name.c_str());
	lua_tographiteval(lua_state_, -1, any_result);

	if(any_result.is_null() && !quiet) {
	    Logger::err("Lua") << name << ": no such global"
			       << std::endl;
	}
	return any_result;
    }

    Any LuaInterpreter::eval(
	const std::string& expression, bool quiet
    ) const {
	geo_argused(quiet);
	Any result;
	lua_pushfstring(lua_state_,"return %s", expression.c_str());
	// Note: luaL_dostring returns false on success (!)
	if(!luaL_dostring(lua_state_, lua_tostring(lua_state_,-1))) {
	    lua_tographiteval(lua_state_,-1,result);
	    lua_pop(lua_state_,1);
	}
	return result;
    }

    void LuaInterpreter::list_names(std::vector<std::string>& names) const {
	names.clear();
	lua_pushglobaltable(lua_state_);
	int index = lua_gettop(lua_state_);
	lua_pushnil(lua_state_);
	while(lua_next(lua_state_,index) != 0) {
	    const char* key = lua_tostring(lua_state_,-2);
	    names.push_back(std::string(key));
	    lua_pop(lua_state_,1);
	}
    }

    void LuaInterpreter::adjust_lua_state() {
    }

    void LuaInterpreter::get_keys(
	const std::string& context, std::vector<std::string>& keys
    ) {
	keys.clear();
	if(context != "") {
	    lua_pushfstring(lua_state_,"return %s", context.c_str());
	    luaL_dostring(lua_state_, lua_tostring(lua_state_,-1));
	    int index = lua_gettop(lua_state_);
	    bool done = false;
	    if(lua_istable(lua_state_, index)) {
		done = true;
		lua_pushnil(lua_state_);
		while(lua_next(lua_state_,index) != 0) {
		    if(lua_isstring(lua_state_,-2)) {
			const char* key = lua_tostring(lua_state_,-2);
			keys.push_back(std::string(key));
		    }
		    lua_pop(lua_state_,1);
		}
	    }
	    lua_pop(lua_state_,1);
	    if(done) {
		return;
	    }
	}
	Interpreter::get_keys(context, keys);
    }

    void LuaInterpreter::display_error_message(const std::string& err) {
	const char* msg = err.c_str();
	const char* msg2 = strchr(msg,']');
	if(msg2 != nullptr) {
	    msg = msg2+2;
	}
	Logger::err("Lua") << msg << std::endl;
    }

    std::string LuaInterpreter::name_value_pair_call(
	const std::string& args
    ) const {
	return '{' + args + '}';
    }
}
