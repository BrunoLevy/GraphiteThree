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
#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/gom/lua/interop.h>
#include <OGF/gom/lua/vec_mat_interop.h>
#include <OGF/gom/types/object.h>
#include <OGF/gom/reflection/meta.h>


extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

namespace OGF {
    namespace GOMLua {

	/*********************************************************************/

	/**
	 * \brief Tests whether an object in the Lua stack is a name-value table
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] index the index of the object in the lua stack
	 * \retval true if the object is a table and all keys are strings
	 * \retval false otherwise
	 */
	static bool is_namevaluetable(lua_State* L, int index) {
	    if(!lua_istable(L,index)) {
		return false;
	    }
	    bool all_keys_are_strings = true;
	    lua_pushnil(L);
	    while(lua_next(L,index) != 0) {
		if(lua_type(L,-2) != LUA_TSTRING) {
		    all_keys_are_strings = false;
		}
		lua_pop(L,1);
	    }
	    return all_keys_are_strings;
	}

	/**
	 * \brief Converts LUA arguments to a graphite ArgList.
	 * \param[in] L a pointer to the LUA state.
	 * \param[out] args a modifiable reference to the graphite ArgList.
	 * \param[in] index the first stack index of the LUA arguments
	 *  to be converted.
	 * \param[in] method an optional pointer to the graphite MetaMethod
	 *  that will be called. It is used to get argument names when using
	 *  standard calls (without name-value pairs).
	 */
	static void lua_tographiteargs(
	    lua_State* L, ArgList& args, int index, MetaMethod* method = nullptr
	) {
	    args.clear();
	    if(is_namevaluetable(L,index)) { // Name-value pairs call
		lua_pushnil(L);
		while(lua_next(L,index) != 0) {
		    // Note: index is already a string, so we can call
		    // lua_tostring, (else it is forbidden, since it would
		    // modify the key in-place, see LUA API doc for more details)
		    //
		    // TODO: we could check for numeric keys, and convert to a
		    // standard call by pushing the args onto the stack and going
		    // to the 'else if' branch of this test. At least we need
		    // to check for non-string keys and send a LUA error, because
		    // users may confuse standard calls
		    // and calls with ({ a,b,c }).
		    const char* key = lua_tostring(L,-2);
		    MetaType* mtype = nullptr;
		    if(method != nullptr) {
			const MetaArg* marg = method->find_arg(key);
			if(marg != nullptr) {
			    mtype = marg->type();
			}
		    }
		    lua_tographiteval(L, -1, args.create_arg(key), mtype);
		    lua_pop(L,1);
		}
	    } else if(method != nullptr) { // Standard call,
		                          // arguments from index to end of stack
		for(index_t i=0; i<method->nb_args(); ++i) {
		    std::string name = method->ith_arg_name(i);
		    if(index + int(i) <= lua_gettop(L)) {
			lua_tographiteval(
			    L, index + int(i),
			    args.create_arg(name), method->ith_arg_type(i)
			);
		    } else {
			if(method->ith_arg_has_default_value(i)) {
			    args.create_arg(
				name,
				method->ith_arg_default_value(i)
			    );
			} else {
			    Logger::warn("GOMLua")
				<< method->container_meta_class()->name()
				<< "::" << method->name() << "()"
				<< " missing argument:"
				<< method->ith_arg_name(i)
				<< std::endl;
			}
		    }
		}
	    } else {
		// Standard call, no meta-method specified
		// -> create an ArgList with all the arguments
		// (used by methods with a single argument of
		// type ArgList, e.g., Interpreter::create()
		for(index_t i=0; index + int(i) <= lua_gettop(L); ++i) {
		    lua_tographiteval(
			L, index + int(i),
			args.create_unnamed_arg()
		    );
		}
	    }
	}

	/**
	 * \brief Implementation of __index() metamethod for graphite objects
	 *  with array indexing.
	 * \details Routes the call to get_element(index_t index)
	 *  method of target object.
	 * \param[in] L a pointer to the LUA state.
	 * \return the number of LUA objects pushed onto the stack (here 1).
	 */
	static int graphite_array_index(lua_State* L) {
	    geo_debug_assert(lua_isgraphite(L,1));
	    Object* object = lua_tographite(L,1);
	    if(object == nullptr) {
		return luaL_error(L, "tried to index nil Graphite object");
	    }
	    Any result;
	    if(lua_isinteger(L,2)) {
		index_t index = index_t(lua_tointeger(L,2));
		object->get_element(index,result);
	    } else {
		vec2i index;
		if(lua_to_graphite_vec2i(L,2,index)) {
		    object->get_element(
			index_t(index[0]), index_t(index[1]), result
		    );
		}
	    }
	    lua_pushgraphiteval(L,result);
	    return 1;
	}

	/**
	 * \brief Implementation of __index() metamethod for graphite objects.
	 * \details Routes attribute lookup to GOM. If index is an integer,
	 *  uses graphite_array_index().
	 * \param[in] L a pointer to the LUA state.
	 * \return the number of LUA objects pushed onto the stack.
	 */
	static int graphite_index(lua_State* L) {
	    geo_debug_assert(lua_isgraphite(L,1));

	    if(lua_type(L,2) != LUA_TSTRING) {
		vec2i index;
		if(
		    lua_type(L,2) == LUA_TNUMBER ||
		    lua_to_graphite_vec2i(L,2,index)
		) {
		    return graphite_array_index(L);
		} else {
		    return luaL_error(L, "attribute name is not a string");
		}
	    }

	    Object* object = lua_tographite(L,1);
	    const char* name = lua_tostring(L,2);

	    if(object == nullptr) {
		return luaL_error(L, "tried to get attribute from nil object");
	    }

	    // Particular case: interfaces scope.
	    if(!strcmp(name,"I")) {
		lua_pushgraphite(L,new InterfaceScope(object));
		return 1;
	    }

	    // Case 1: regular property
	    MetaProperty* mprop = object->meta_class()->find_property(name);
	    if(mprop != nullptr) {
		Any value;
		if(!object->get_property(name, value)) {
		    return luaL_error(
			L,(
			    object->meta_class()->name() +
			    "::" + name + " : could not get property"
			).c_str()
		    );
		}
		lua_pushgraphiteval(L,value);
		return 1;
	    }

	    // Case 2: assembling a graphite request (object.method to be
	    // called right after).
	    MetaMethod* mmethod = object->meta_class()->find_method(name);
	    if(
		mmethod != nullptr &&
		dynamic_cast<MetaConstructor*>(mmethod) == nullptr
	    ) {
		// If object is an interpreter, do not do reference counting,
                // else this creates circular references, preventing objects from
		// being deallocated.
		bool managed = (dynamic_cast<Interpreter*>(object) == nullptr);
		lua_pushgraphite(L, new Request(object,mmethod,managed));
		return 1;
	    }

	    // Case 3: resolving symbol in gom.globals
	    {
		Scope* scope = dynamic_cast<Scope*>(object);
		if(scope != nullptr) {
		    Any prop = scope->resolve(name);
		    lua_pushgraphiteval(L,prop);
		    return 1;
		}
	    }

	    return luaL_error(
		L,(
		    object->meta_class()->name() +
		    "::" + name + " : no such method nor property"
		).c_str()
	    );
	}

	/**
	 * \brief Implementation of __newindex() metamethod for graphite objects
	 *  with array indexing.
	 * \details Routes the call to set_element(index_t index, T value) method
	 *  of target object.
	 * \param[in] L a pointer to the LUA state.
	 * \return the number of LUA objects pushed onto the stack (here 0).
	 */
	static int graphite_array_newindex(lua_State* L) {
	    geo_debug_assert(lua_isgraphite(L,1));
	    geo_debug_assert(lua_isinteger(L,2));
	    Object* object = lua_tographite(L,1);
	    if(object == nullptr) {
		return luaL_error(L, "tried to index nil Graphite object");
	    }

	    Any value;
	    lua_tographiteval(L,3,value);

	    if(lua_type(L,2) == LUA_TNUMBER) {
		index_t index = index_t(lua_tointeger(L,2));
		object->set_element(index, value);
	    } else {
		vec2i index;
		if(lua_to_graphite_vec2i(L,2,index)) {
		    object->set_element(
			index_t(index[0]), index_t(index[1]), value
		    );
		}
	    }
	    return 0;
	}

	/**
	 * \brief Implementation of __newindex() metamethod for graphite objects.
	 * \details Routes attribute set to GOM or to graphite_array_newindex()
	 *  if index is an integer.
	 * \param[in] L a pointer to the LUA state.
	 * \return the number of LUA objects pushed onto the stack.
	 */
	static int graphite_newindex(lua_State* L) {
	    geo_debug_assert(lua_isgraphite(L,1));

	    {
		vec2i index;
		if(
		    lua_type(L,2) == LUA_TNUMBER ||
		    lua_to_graphite_vec2i(L,2,index)
		) {
		    return graphite_array_newindex(L);
		}
	    }

	    if(!lua_isstring(L,2)) {
		return luaL_error(L, "attribute name is not a string");
	    }

	    Object* object = lua_tographite(L,1);
	    const char* key = lua_tostring(L,2);
	    if(object == nullptr) {
		return luaL_error(L, "tried to get attribute from nil object");
	    }
	    Any value;
	    MetaType* mtype = nullptr;
	    MetaProperty* mprop = object->meta_class()->find_property(key);
	    if(mprop != nullptr) {
		mtype = mprop->type();
	    }
	    lua_tographiteval(L,3,value,mtype);
	    object->set_property(key,value);
	    return 0;
	}


	/**
	 * \brief Implementation of __len() metamethod for graphite objects
	 *  with array indexing.
	 * \details Routes the call to nb_elements() method
	 *  of target object.
	 * \param[in] L a pointer to the LUA state.
	 * \return the number of LUA objects pushed onto the stack (here 1).
	 */
	static int graphite_array_len(lua_State* L) {
	    geo_debug_assert(lua_isgraphite(L,1));
	    Object* object = lua_tographite(L,1);
	    if(object == nullptr) {
		return luaL_error(L, "tried to index nil Graphite object");
	    }
	    lua_pushinteger(L, lua_Integer(object->get_nb_elements()));
	    return 1;
	}

	/**
	 * \brief Implementation of __gc() metamethod for graphite objects.
	 * \param[in] L a pointer to the LUA state.
	 * \return the number of LUA objects pushed onto the stack.
	 */
	static int graphite_gc(lua_State* L) {
	    geo_debug_assert(lua_isgraphite(L,1));
	    GraphiteRef* GR = static_cast<GraphiteRef*>(
		lua_touserdata(L,1)
	    );
	    if(GR->managed && GR->object != nullptr) {
		GR->object->unref();
	    }
	    GR->object = nullptr;
	    return 0;
	}

	/**
	 * \brief Implementation of __call() metamethod for graphite callables.
	 * \details Routes the call to GOM.
	 * \param[in] L a pointer to the LUA state.
	 * \return the number of LUA objects pushed onto the stack.
	 */
	static int graphite_call(lua_State* L) {
	    if(!lua_isgraphite(L,1)) {
		geo_assert_not_reached;
	    }
	    Callable* c = dynamic_cast<Callable*>(lua_tographite(L,1));
	    if(c == nullptr) {
		return luaL_error(
		    L, "Error in graphite_call(): no callable target"
		);
	    }

	    Request* r = dynamic_cast<Request*>(c);
	    if(r != nullptr) {
		ArgList args;
		// Special case: method has a single argument of type ArgList
		// -> pack all the arguments in a ArgList
		if(
		    r->method()->nb_args() == 1 &&
		    r->method()->ith_arg_type(0) ==
		    ogf_meta<OGF::ArgList>::type()
		) {
		    lua_tographiteargs(L,args,2);
		} else {
		    // Regular case: identify each individual argument according
		    // to method declaration. Add default values if need be.
		    lua_tographiteargs(L,args,2,r->method());
		}
		Any result;
		bool ok = r->invoke(args, result);
		if(!ok) {
		    return luaL_error(
			L,(
			    "GOM error while invoking " +
			    r->object()->meta_class()->name() +
			    "::" + r->method()->name()
			).c_str()
		    );
		}
		lua_pushgraphiteval(L, result);
		return 1;
	    }

	    Any result;
	    ArgList args;
	    lua_tographiteargs(L,args,2);
	    c->invoke(args, result);
	    if(
		!result.is_null() &&
		result.meta_type() != ogf_meta<void>::type()
	    ) {
		lua_pushgraphiteval(L,result);
		return 1;
	    } else {
		return 0;
	    }
	}

	/*********************************************************************/

	void init_lua_graphite(LuaInterpreter* interpreter) {
	    lua_State* L = interpreter->lua_state();

	    // Create the "metatable" for
	    // graphite objects.
	    {
		lua_newtable(L);

		// Attribute access (read)
		lua_pushliteral(L,"__index");
		lua_pushcfunction(L,graphite_index);
		lua_settable(L,-3);

		// Attribute access (write)
		lua_pushliteral(L,"__newindex");
		lua_pushcfunction(L,graphite_newindex);
		lua_settable(L,-3);

		// Length
		lua_pushliteral(L,"__len");
		lua_pushcfunction(L,graphite_array_len);
		lua_settable(L,-3);

		// Garbage collection of Graphite objects
		lua_pushliteral(L,"__gc");
		lua_pushcfunction(L,graphite_gc);
		lua_settable(L,-3);

		// Function call
		lua_pushliteral(L,"__call");
		lua_pushcfunction(L,graphite_call);
		lua_settable(L,-3);

		lua_setfield(L, LUA_REGISTRYINDEX, "graphite_vtbl");
	    }

	    // Create the global table for gom2lua connections
	    {
		lua_newtable(L);
		lua_setfield(L, LUA_REGISTRYINDEX, "graphite_lua_targets");
	    }

	    // Last argument set to false: do not manage ref count, else
	    // this would generate a circular reference.
	    lua_pushgraphite(L, interpreter, false);
	    lua_setglobal(L, "gom");
	}

	/********************************************************************/

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
