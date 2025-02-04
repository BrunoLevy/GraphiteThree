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

#ifndef H_OGF_GOM_LUA_INTEROP_H
#define H_OGF_GOM_LUA_INTEROP_H

/**
 * \file OGF/gom/lua/interop.h
 * \brief Functions to exchange objects between Lua and Graphite
 */

struct lua_State;

namespace OGF {

    class Any;
    class MetaType;

    namespace GOMLua {

	/**
	 * \brief Pushes a value on the LUA stack.
	 * \details The GOM meta type \p mtype is used to determine
	 *  the type of the LUA value pushed onto the stack.
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] value the value represented by an Any
	 */
	void lua_pushgraphiteval(lua_State* L, const Any& value);

	/**
	 * \brief Converts a LUA value to a graphite value stored
	 *  in an Any.
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] index the stack index of the LUA object to be converted.
	 * \param[out] result the result wrapped in an Any
	 * \param[in] mtype the optional desired metatype
	 */
	void lua_tographiteval(
	    lua_State* L, int index, Any& result, MetaType* mtype = nullptr
	);
    }
}

#endif
