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

#include <OGF/gom/lua/interop.h>
#include <OGF/gom/lua/lua_graphite_object.h>
#include <OGF/gom/lua/lua_callable.h>
#include <OGF/gom/lua/vec_mat_interop.h>
#include <OGF/gom/reflection/meta.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

namespace OGF {
    namespace GOMLua {

	void lua_pushgraphiteval(lua_State* L, const Any& value) {
	    if(value.is_null()) {
		lua_pushnil(L);
		return;
	    }

	    MetaType* mtype = value.meta_type();

	    if(Any::is_pointer_type(mtype)) {
		MetaType* mbasetype = Any::pointed_type(mtype);
		if(dynamic_cast<MetaClass*>(mbasetype) != nullptr) {
		    Object* object = nullptr;
		    value.get_value(object);
		    lua_pushgraphite(L,object);
		    return;
		} else {
		    void* ptr = nullptr;
		    value.get_value(ptr);
		    lua_pushlightuserdata(L,ptr);
		    return;
		}
	    }

	    if(mtype == ogf_meta<void>::type()) {
		lua_pushnil(L);
		return;
	    }

	    if(mtype == ogf_meta<bool>::type()) {
		bool val;
		value.get_value(val);
		lua_pushboolean(L, val ? 1 : 0);
		return;
	    }

	    if(mtype == ogf_meta<int>::type()) {
		int val;
		value.get_value(val);
		lua_pushinteger(L,lua_Integer(val));
		return;
	    }

	    if(mtype == ogf_meta<unsigned int>::type()) {
		unsigned int val;
		value.get_value(val);
		lua_pushinteger(L,lua_Integer(val));
		return;
	    }

	    if(mtype == ogf_meta<long>::type()) {
		long val;
		value.get_value(val);
		lua_pushinteger(L,lua_Integer(val));
		return;
	    }

	    if(mtype == ogf_meta<unsigned long>::type()) {
		unsigned long val;
		value.get_value(val);
		lua_pushinteger(L,lua_Integer(val));
		return;
	    }

	    if(mtype == ogf_meta<index_t>::type()) {
		index_t val;
		value.get_value(val);
		lua_pushinteger(L,lua_Integer(val));
		return;
	    }

	    if(mtype == ogf_meta<signed_index_t>::type()) {
		signed_index_t val;
		value.get_value(val);
		lua_pushinteger(L,lua_Integer(val));
		return;
	    }

	    if(mtype == ogf_meta<float>::type()) {
		float val;
		value.get_value(val);
		lua_pushnumber(L, lua_Number(val));
		return;
	    }

	    if(mtype == ogf_meta<double>::type()) {
		double val;
		value.get_value(val);
		lua_pushnumber(L, lua_Number(val));
		return;
	    }

	    if(mtype == ogf_meta<size_t>::type()) {
		size_t val;
		value.get_value(val);
		lua_pushnumber(L, lua_Number(val));
		return;
	    }

	    if(GOMLua::push_mat_vec(L, value)) {
		return;

	    }

	    std::string as_string = value.as_string();
	    lua_pushstring(L,as_string.c_str());
	}

	void lua_tographiteval(
	    lua_State* L, int index, Any& result, MetaType* mtype
	) {
	    if(mtype != nullptr) {
		// Test arguments of type pointer to gom_class,
		// and verify that it is really an instance of a gom_class.
		if(Any::is_pointer_type(mtype)) {
		    mtype = Any::pointed_type(mtype);
		    if(dynamic_cast<MetaClass*>(mtype) != nullptr) {
			if(lua_isgraphite(L,index)) {
			    Object* object = lua_tographite(L,index);
			    result.set_value(object);
			    return;
			} else {
			    if(mtype == ogf_meta<OGF::Callable>::type()) {
				LuaCallable* lua_callable =
				    new LuaCallable(L, index);
				result.set_value(lua_callable);
				return;
			    } else {
				const char* s = lua_tostring(L,index);
				if(s != nullptr) {
				    Logger::warn("GOMLua")
					<< "Expected Object, got "
					<< (s ? s : "NULL") << " instead"
					<< std::endl;
				}
				result.reset();
				return;
			    }
			}
		    }
		}
		if(lua_to_graphite_mat_vec(L, index, result, mtype)) {
		    return;
		}
	    }

	    // Note: for boolean, number and string, we use
	    // lua_type(), because the lua_isxxx() functions
	    // test whether it can be converted into an xxx
	    // (instead of whether it is an xxx), misleading !

	    if(lua_isfunction(L,index)) {
		LuaCallable* lua_callable = new LuaCallable(L, index);
		result.set_value(lua_callable);
		return;
	    } else if(lua_islightuserdata(L,index)) {
		result.set_value(lua_touserdata(L,index));
		return;
	    } else if(lua_isgraphite(L,index)) {
		Object* object = lua_tographite(L,index);
		result.set_value(object);
		return;
	    } else if(lua_type(L,index) == LUA_TBOOLEAN) {
		result.set_value(lua_toboolean(L,index) != 0);
		return;
	    } else if(lua_type(L,index) == LUA_TNUMBER) {
		if(lua_isinteger(L,index)) {
		    // We convert to int because LUA integers are not
		    // registered to GOM.
		    result.set_value(int(lua_tointeger(L,index)));
		} else {
		    result.set_value(lua_tonumber(L,index));
		}
		return;
	    } else if(lua_type(L,index) == LUA_TSTRING) {
		result.set_value(std::string(lua_tostring(L,index)));
		return;
	    } else if(lua_isnil(L,index)) {
		result.reset();
		return;
	    }
	    result.reset();
	}
    }
}
