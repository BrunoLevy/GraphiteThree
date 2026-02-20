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

#include <OGF/gom/lua/lua_callable.h>
#include <OGF/gom/lua/lua_graphite_object.h>
#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/gom/lua/interop.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

namespace OGF {
    using namespace OGF::GOMLua;

    LuaCallable::LuaCallable(lua_State* L, int target_index) {
	geo_lua_check_stack(L);
	lua_state_ = L;

	// Since we change the stack, we need to convert index relative
	// to top into absolute index.
	if(target_index < 0) {
	    target_index = lua_gettop(lua_state_) + target_index + 1;
	}
	++current_instance_id_;
	instance_id_ = current_instance_id_;

	// Memorize a copy of the target in the "graphite_lua_targets"
	// table (indexed by instance_id_)
	lua_getfield(lua_state_,LUA_REGISTRYINDEX,"graphite_lua_targets");
	lua_pushnil(lua_state_);
	lua_copy(lua_state_, target_index, -1);
	lua_seti(lua_state_, -2, lua_Integer(instance_id_));
        lua_pop(lua_state_, 1); // Leave the Lua stack as we found it

        // Keep a ref to the Lua interpreter so that the Lua interpreter
        // is not destroyed before the last LuaCallable closes the door.
        interpreter_ = Interpreter::instance_by_language("Lua");
    }


    static bool args_are_for_name_value_call(const ArgList& args) {
        if(args.nb_args() == 0) {
            return false;
        }
	if(args.nb_args() == 1 && args.ith_arg_name(0) == "value") {
	    return false;
	}
        return !args.has_unnamed_args();
    }

    bool LuaCallable::invoke(const ArgList& args, Any& ret_val) {
	geo_lua_check_stack(lua_state_);
	ret_val.reset();
	bool result = true;

	// Get the memorized target from the "graphite_lua_targets"
	// table (indexed by instance_id_)
	lua_getfield(lua_state_,LUA_REGISTRYINDEX,"graphite_lua_targets");
	lua_geti(lua_state_, -1, lua_Integer(instance_id_));

	if(args_are_for_name_value_call(args)) {
	    lua_newtable(lua_state_);
	    for(index_t i=0; i<args.nb_args(); ++i) {
		lua_pushgraphiteval(lua_state_, args.ith_arg_value(i));
		lua_setfield(lua_state_, -2, args.ith_arg_name(i).c_str());
	    }
	    result = (lua_pcall(lua_state_, 1, 1, 0) == 0);
	} else {
	    for(index_t i=0; i<args.nb_args(); ++i) {
		lua_pushgraphiteval(lua_state_, args.ith_arg_value(i));
	    }
	    result = (
		lua_pcall(lua_state_, int(args.nb_args()), 1, 0) == 0
	    );
	}

	if(result) {
	    lua_tographiteval(lua_state_,-1,ret_val);
	    lua_pop(lua_state_,2);
	} else {
	    std::string err = std::string(lua_tostring(lua_state_, -1));
	    lua_pop(lua_state_,2);
	    LuaInterpreter::display_error_message(err);
	}
	return result;
    }

    LuaCallable::~LuaCallable() {
	geo_lua_check_stack(lua_state_);
	// Remove the memorized target from the "graphite_lua_targets"
	// table (indexed by instance_id_)
	lua_getfield(lua_state_,LUA_REGISTRYINDEX,"graphite_lua_targets");
	lua_pushnil(lua_state_);
	lua_seti(lua_state_, -2, lua_Integer(instance_id_));
	lua_pop(lua_state_,1);
    }

    index_t LuaCallable::current_instance_id_ = 0;
}
