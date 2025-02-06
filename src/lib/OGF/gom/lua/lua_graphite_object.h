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

#ifndef H_OGF_GOM_LUA_GRAPHITE_OBJECT_H
#define H_OGF_GOM_LUA_GRAPHITE_OBJECT_H

/**
 * \file OGF/gom/lua/lua_graphite_object.h
 * \brief Lua wrapper around a Graphite object.
 */

#include <OGF/gom/common/common.h>

struct lua_State;

namespace OGF {

    class Object;
    class LuaInterpreter;

    namespace GOMLua {

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
	 * \brief Tests whether a LUA object is a graphite object.
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] index the stack index of the LUA object to be tested.
	 * \retval true if the LUA object at \p index is a graphite object.
	 * \retval false otherwise.
	 */
	bool lua_isgraphite(lua_State* L, int index);

	/**
	 * \brief Gets a pointer to a graphite object from a
	 *  LUA object.
	 * \pre lua_isgraphite(L,index)
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] index the stack index of the LUA object to be tested.
	 * \return a pointer to the graphite object.
	 */
	Object* lua_tographite(lua_State* L, int index);

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
	);

	/**
	 * \brief Initializes the LUA state for graphite.
	 * \details Registers GOM implementation functions,
	 *  creates the metatables for graphite objects and
	 *  graphite requests, and create the global table
	 *  for LUA targets. The global tables are created in the
	 *  registry.
	 * \param[in] interpreter a pointer to the LuaInterpreter.
	 */
	void init_lua_graphite(LuaInterpreter* interpreter);
    }
}

#endif
