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

namespace {
    using namespace OGF;
    
    void lua_pushgraphiteval(lua_State* L, const Any& value);
    void lua_tographiteval(
	lua_State* L, int index, Any& result, MetaType* mtype = nullptr
    );
}

/*************************************************************************/

namespace OGF {

    LuaCallable::LuaCallable(lua_State* L, int target_index) {
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
	} else {
	    std::string err = std::string(lua_tostring(lua_state_, -1));
	    lua_pop(lua_state_,1);
	    LuaInterpreter::display_error_message(err);
	}
	return result;
    }

    LuaCallable::~LuaCallable() {
	// Remove the memorized target from the "graphite_lua_targets"
	// table (indexed by instance_id_)
	lua_getfield(lua_state_,LUA_REGISTRYINDEX,"graphite_lua_targets");
	lua_pushnil(lua_state_);
	lua_seti(lua_state_, -2, lua_Integer(instance_id_));	    
    }

    index_t LuaCallable::current_instance_id_ = 0;
}


/*****************************************************************************/


namespace {
    using namespace OGF;

    /**
     * \brief Tests whether a LUA object is a graphite object.
     * \param[in] L a pointer to the LUA state.
     * \param[in] index the stack index of the LUA object to be tested.
     * \retval true if the LUA object at \p index is a graphite object.
     * \retval false otherwise.
     */
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

    /**
     * \brief Representation of a pointer to a Graphite object
     *  in the LUA interpreter.
     * \details Contains a pointer to the Graphite object and a
     *  boolean that indicates whether reference counting should
     *  be done for the object. This is needed because the interpreter
     *  itself is a Graphite object, and is accessible through the "gom"
     *  global variable. If reference counting is enabled, then we got
     *  a circular reference that prevents the interpreter from being
     *  deallocated on exit.
     */
    struct GraphiteRef {
	Object* object;
	bool managed;
    };
    
    /**
     * \brief Pushes a graphite object onto the LUA stack.
     * \details Graphite objects in LUA are seen as a
     *  full user data that contains a pointer to the
     *  object. We use full user data (rather than
     *  light user data) because full user data can
     *  have a metatable, that lets us redirect
     *  member access to the GOM system.
     * \param[in] L a pointer to the LUA state.
     * \param[in] object a pointer to the graphite object
     * \param[in] managed if true, manages reference count, else
     *  only store a reference to the object without changing the reference
     *  count. This is needed for the Interpreter itself, accessible through
     *  the "gom" global variable. If it was reference-counted, then we would
     *  have a circular reference, preventing the interpreter from being 
     *  deallocated on exit.
     */
    void lua_pushgraphite(
	lua_State* L, Object* object, bool managed=true
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

    /**
     * \brief Gets a pointer to a graphite object from a 
     *  LUA object.
     * \pre lua_isgraphite(L,index)
     * \param[in] L a pointer to the LUA state.
     * \param[in] index the stack index of the LUA object to be tested.
     * \return a pointer to the graphite object.
     */
    Object* lua_tographite(lua_State* L, int index) {
	geo_debug_assert(lua_isgraphite(L,index));
	return static_cast<GraphiteRef*>(
	    lua_touserdata(L,index)
	)->object;
    }

    /***************************************************/
    
    /**
     * \brief Converts a LUA value to a graphite value stored
     *  in an Any.
     * \param[in] L a pointer to the LUA state.
     * \param[in] index the stack index of the LUA object to be converted.
     * \param[out] result the result wrapped in an Any
     * \param[in] mtype the optional desired metatype 
     */
    void lua_tographiteval(
	lua_State* L, int index, Any& result, MetaType* mtype 
    ) {

	// Test arguments of type pointer to gom_class, and verify that it is
	// really an instance of a gom_class.
	if(mtype != nullptr) {
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
			    Logger::warn("GOMLua")
				<< "Expected Object, got "
				<< lua_tostring(L,index) << " instead"
				<< std::endl;
			    result.reset();
			    return;
			}
		    }
		}
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
    void lua_tographiteargs(
	lua_State* L, ArgList& args, int index, MetaMethod* method = nullptr
    ) {
	args.clear();
	if(lua_istable(L,index)) { // Name-value pairs call
	    lua_pushnil(L);
	    while(lua_next(L,index) != 0) {
		// Note: index is already a string, so we can call lua_tostring,
		//(else it is forbidden, since it would modify the key in-place,
		//see LUA API doc for more details)
		//
		// TODO: we could check for numeric keys, and convert to a
		// standard call by pushing the args onto the stack and going
		// to the 'else if' branch of this test. At least we need
		// to check for non-string keys and send a LUA error, because
		// users may confuse standard calls and calls with ({ a,b,c }).
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
     * \brief Pushes a value on the LUA stack.
     * \details The GOM meta type \p mtype is used to determine
     *  the type of the LUA value pushed onto the stack.
     * \param[in] L a pointer to the LUA state.
     * \param[in] value the value represented by an Any
     */
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

	std::string as_string = value.as_string();
	lua_pushstring(L,as_string.c_str());
    }

    /**
     * \brief Implementation of __index() metamethod for graphite objects
     *  with array indexing.
     * \details Routes the call to get_element(index_t index) method of target
     *  object.
     * \param[in] L a pointer to the LUA state.
     * \return the number of LUA objects pushed onto the stack (here 1).
     */
    int graphite_array_index(lua_State* L) {
	geo_debug_assert(lua_isgraphite(L,1));
	geo_debug_assert(lua_isinteger(L,2));
	Object* object = lua_tographite(L,1);
	index_t index = index_t(lua_tointeger(L,2));
	if(object == nullptr) {
	    return luaL_error(L, "tried to index nil Graphite object");
	}
	Any result;
	object->get_element(index,result);
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
    int graphite_index(lua_State* L) {
	geo_debug_assert(lua_isgraphite(L,1));

	if(lua_isinteger(L,2)) {
	    return graphite_array_index(L);
	}
	
	if(!lua_isstring(L,2)) {
	    return luaL_error(L, "attribute name is not a string");	    
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
	    // If object is an interpreter, do not do reference counting, else
	    // this creates circular references, preventing objects from
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
    int graphite_array_newindex(lua_State* L) {
	geo_debug_assert(lua_isgraphite(L,1));
	geo_debug_assert(lua_isinteger(L,2));
	Object* object = lua_tographite(L,1);
	if(object == nullptr) {
	    return luaL_error(L, "tried to index nil Graphite object");
	}
	index_t index = index_t(lua_tointeger(L,2));
	Any value;
	lua_tographiteval(L,3,value);
	object->set_element(index, value);
	return 0;
    }
    
    /**
     * \brief Implementation of __newindex() metamethod for graphite objects.
     * \details Routes attribute set to GOM or to graphite_array_newindex() if
     *  index is an integer.
     * \param[in] L a pointer to the LUA state.
     * \return the number of LUA objects pushed onto the stack.
     */
    int graphite_newindex(lua_State* L) {
	geo_debug_assert(lua_isgraphite(L,1));

	if(lua_isinteger(L,2)) {
	    return graphite_array_newindex(L);
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
	lua_tographiteval(L,3,value);
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
    int graphite_array_len(lua_State* L) {
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
    int graphite_gc(lua_State* L) {
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
    int graphite_call(lua_State* L) {
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
		r->method()->ith_arg_type(0) == ogf_meta<OGF::ArgList>::type()
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

    /**
     * \brief Initializes the LUA state for graphite.
     * \details Registers GOM implementation functions,
     *  creates the metatables for graphite objects and
     *  graphite requests, and create the global table
     *  for LUA targets. The global tables are created in the
     *  registry.
     * \param[in] interpreter a pointer to the LuaInterpreter.
     */
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
    
}

/*************************************************************************/

namespace OGF {
    
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
