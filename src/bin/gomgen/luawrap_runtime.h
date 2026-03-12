/*
 *  Copyright (c) 2000-2022 Inria
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 */

#ifndef LUAWRAP_RUNTIME
#define LEAWRAP_RUNTIME

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GEOGRAM_USE_BUILTIN_DEPS
#include <geogram/third_party/lua/lua.h>
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
#else
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#endif

#ifdef __cplusplus
}
#endif

#ifdef GEOGRAM_USE_BUILTIN_DEPS
#include <geogram_gfx/third_party/imgui/imgui.h>
#else
#include <imgui.h>
#endif

#include <vector>
#include <string>


namespace LuaWrap {

    template <class T> struct LuaType {
    };

    template <> struct LuaType<lua_Integer> {
	static bool check(lua_State* L, int idx) {
	    return lua_isinteger(L,idx);
	}
	static lua_Integer get(lua_State* L, int idx) {
	    return lua_tointeger(L, idx);
	}
	template <class T> static void push(lua_State* L, T val) {
	    lua_pushinteger(L,lua_Integer(val));
	}
    };

    template <> struct LuaType<lua_Number> {
	static bool check(lua_State* L, int idx) {
	    return lua_isnumber(L,idx);
	}
	static lua_Number get(lua_State* L, int idx) {
	    return lua_tonumber(L, idx);
	}
	template <class T> static void push(lua_State* L, T val) {
	    lua_pushinteger(L,lua_Number(val));
	}
    };

    template <> struct LuaType<const char*> {
	static bool check(lua_State* L, int idx) {
	    return lua_isstring(L,idx);
	}
	static const char* get(lua_State* L, int idx) {
	    return lua_tostring(L, idx);
	}
	template <class T> static void push(lua_State* L, T val) {
	    lua_pushstring(L,(const char*)(val));
	}
    };

    template <> struct LuaType<bool> {
	static bool check(lua_State* L, int idx) {
	    return lua_isboolean(L,idx);
	}
	static bool get(lua_State* L, int idx) {
	    return lua_toboolean(L, idx);
	}
	template <class T> static void push(lua_State* L, T val) {
	    lua_pushboolean(L,bool(val));
	}
    };

    template <> struct LuaType<ImVec2> {
	static bool check(lua_State* L, int idx) {
	    return (lua_gettop(L) >= idx+1) &&
		lua_isnumber(L,idx) && lua_isnumber(L,idx+1);
	}
	static ImVec2 get(lua_State* L, int idx) {
	    return ImVec2(lua_tonumber(L,idx),lua_tonumber(L,idx+1));
	}
	static void push(lua_State* L, ImVec2 val) {
	    lua_pushnumber(L,lua_Number(val.x));
	    lua_pushnumber(L,lua_Number(val.y));
	}
    };

    template <> struct LuaType<ImVec4> {
	static bool check(lua_State* L, int idx) {
	    return (lua_gettop(L) >= idx+3) &&
		lua_isnumber(L,idx  ) && lua_isnumber(L,idx+1) &&
		lua_isnumber(L,idx+2) && lua_isnumber(L,idx+3) ;
	}
	static ImVec4 get(lua_State* L, int idx) {
	    return ImVec4(
		lua_tonumber(L,idx  ),lua_tonumber(L,idx+1),
		lua_tonumber(L,idx+2),lua_tonumber(L,idx+3)
	    );
	}
	static void push(lua_State* L, ImVec4 val) {
	    lua_pushnumber(L,lua_Number(val.x));
	    lua_pushnumber(L,lua_Number(val.y));
	    lua_pushnumber(L,lua_Number(val.z));
	    lua_pushnumber(L,lua_Number(val.w));
	}
    };

    enum UninitializedPointer { };
    enum NullPointer { };

    struct ArgBase {
	enum State { UNINITIALIZED, INITIALIZED, SET, INVALID };
	ArgBase() : state(UNINITIALIZED) { }
	bool OK() const {
	    return (state != UNINITIALIZED) && (state != INVALID);
	}
	State state;
	bool is_pointer;
    };

    template <class CTYPE, class LUATYPE=CTYPE> struct Arg : public ArgBase {
	Arg(lua_State* L, int idx) {
	    get(L, idx);
	}

	Arg(lua_State* L, int idx, CTYPE val) {
	    value = val;
	    state = INITIALIZED;
	    get(L, idx);
	}

	Arg(lua_State* L, int idx, NullPointer) {
	    state = INITIALIZED;
	    is_pointer = true;
	    get(L, idx);
	}

	Arg(lua_State* L, int idx, UninitializedPointer) {
	    state = UNINITIALIZED;
	    is_pointer = true;
	    get(L, idx);
	}

	Arg(CTYPE val) {
	    value = val;
	    state = INITIALIZED;
	}

	void push(lua_State* L) {
	    LuaType<LUATYPE>::push(L,value);
	}

	void push_pointer_if_set(lua_State* L) {
	    assert(is_pointer);
	    if(state == SET) {
		LuaType<LUATYPE>::push(L,value);
	    }
	}

	CTYPE value;
    protected:
	void get(lua_State* L, int idx) {
	    if(lua_isnoneornil(L,idx)) {
		return;
	    }
	    if(LuaType<LUATYPE>::check(L,idx)) {
		value = CTYPE(LuaType<LUATYPE>::get(L,idx));
		state = SET;
	    } else {
		state = INVALID;
	    }
	}
    };

    inline std::pair<bool, std::string> check_args(
	const std::vector<ArgBase>& args
    ) {
	bool OK = true;
	for(const ArgBase& arg: args) {
	    OK = OK && arg.OK();
	}
	if(OK) {
	    return std::make_pair(true, std::string());
	}
	// there was an error, collect the problematic args
	std::string missing;
	std::string wrong_type;
	int i = 0;
	for(const ArgBase& arg : args) {
	    if(arg.state == ArgBase::UNINITIALIZED) {
		if(missing == "") {
		    missing = std::to_string(i);
		} else {
		    missing += ("," + std::to_string(i));
		}
	    } else if(arg.state == ArgBase::INVALID) {
		if(wrong_type == "") {
		    wrong_type = std::to_string(i);
		} else {
		    wrong_type += ("," + std::to_string(i));
		}
	    }
	    ++i;
	}
	std::string err_msg =
	    "\nmissing args: " + missing + " wrong type args: " + wrong_type;
	return std::make_pair(false, err_msg);
    }
}


#define LUAWRAP_DECLARE_GLOBAL_CONSTANT(L,C) \
    lua_pushinteger(L,C);                    \
    lua_setglobal(L,#C)

#define LUAWRAP_DECLARE_FUNCTION(L,F) \
    lua_pushliteral(L,#F);  \
    lua_pushcfunction(L,F); \
    lua_settable(L,-3);

#define LUAWRAP_CHECK_ARGS(...) \
   auto [arglist_OK, err_msg] = check_args({__VA_ARGS__}); \
   if(!arglist_OK) \
      return luaL_error(L,(proto+err_msg).c_str())

#endif
