/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
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

#ifndef H_OGF_GOM_LUA_VEC_MAT_INTEROP_H
#define H_OGF_GOM_LUA_VEC_MAT_INTEROP_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/lua/lua_interpreter.h>
#include <geogram/basic/geometry.h>

/**
 * \file OGF/gom/lua/vec_mat_interop.h
 * \brief Functions to exchange vec2,vec3,vec4 and mat4 objects
 *  between Lua and Graphite
 */

namespace OGF {

    class Any;
    class MetaType;

    namespace GOMLua {

	/**
	 * \brief Converts a Lua object into a Graphite vec or mat type
	 * \details Works with vec2, vec3, vec4 of doubles and integers, and
	 *   with mat4 of doubles.
	 * \param[in] L a pointer to the Lua state
	 * \param[in] index the index of the Lua object to be converted
	 * \param[out] result the result, as an Any
	 * \param[in] mtype a pointer to the expected meta-type
	 * \retval true if conversion was successful
	 * \retval false otherwise
	 */
	bool lua_to_graphite_mat_vec(
	    lua_State* L, int index, Any& result, MetaType* mtype
	);

	/**
	 * \brief Converts a Lua object into a Graphite vec2i
	 * \param[in] L a pointer to the Lua state
	 * \param[in] index the index of the Lua object to be converted
	 * \param[out] result a reference to the result, as an vec2i
	 * \retval true if conversion was successful
	 * \retval false otherwise
	 */
	bool lua_to_graphite_vec2i(lua_State* L, int index, vec2i& result);


	/**
	 * \brief Pushes a Graphite object onto the Lua stack
	 * \details Works with vec2, vec3, vec4 of doubles and integers, and
	 *   with mat4 of doubles.
	 * \param[in] matvec the input vec or mat stored in an Any
	 * \retval true if conversion was successful
	 * \retval false otherwise
	 */
	bool push_mat_vec(lua_State* L, const Any& matvec);
    }
}

#endif
