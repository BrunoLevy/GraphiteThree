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

#include <OGF/gom/lua/lua_graphite_object.h>
#include <OGF/gom/types/object.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

namespace OGF {
    namespace GOMLua {

	void lua_pushgraphite(
	    lua_State* L, Object* object, bool managed
	) {
	    if(object == nullptr) {
		lua_pushnil(L);
		return;
	    }
	    void* p = lua_newuserdata(L,sizeof(GraphiteRef));
	    GraphiteRef* GR = static_cast<GraphiteRef*>(p);
	    GR->object = object;
	    GR->managed = managed;
	    if(managed && object != nullptr) {
		GR->object->ref();
	    }
	    lua_getfield(L,LUA_REGISTRYINDEX,"graphite_vtbl");
	    lua_setmetatable(L,-2);
	}

	Object* lua_tographite(lua_State* L, int index) {
	    geo_debug_assert(lua_isgraphite(L,index));
	    return static_cast<GraphiteRef*>(
		lua_touserdata(L,index)
	    )->object;
	}


	bool lua_isgraphite(lua_State* L, int index) {
	    if(lua_islightuserdata(L,index)) {
		return false;
	    }
	    if(!lua_isuserdata(L,index)) {
		return false;
	    }
	    lua_getmetatable(L,index);
	    lua_getfield(L,LUA_REGISTRYINDEX,"graphite_vtbl");
	    bool result = (lua_compare(L, -1, -2, LUA_OPEQ) != 0);
	    lua_pop(L,2);
	    return result;
	}

    }
}
