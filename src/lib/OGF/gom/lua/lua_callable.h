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

#ifndef H_OGF_GOM_LUA_CALLABLE_H
#define H_OGF_GOM_LUA_CALLABLE_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/types/callable.h>

struct lua_State;

/**
 * \file OGF/gom/lua/lua_callable.h
 * \brief Wrapper around a lua callable that can be called from GOM.
 */

namespace OGF {
    /**
     * \brief GOM wrapper around a LUA function.
     */
    gom_class GOM_API LuaCallable : public Callable {
    public:
        /**
         * \brief LuaCallable constructor.
         * \param[in] L a pointer to the Lua state.
	 * \param[in] target_index the stack index where the target
	 *  resides in the stack
         */
        LuaCallable(lua_State* L, int target_index);

        /**
         * \copydoc Callable::invoke()
         */
        bool invoke(const ArgList& args, Any& ret_val) override;

	/**
	 * \brief LuaCallable destructor.
	 */
	~LuaCallable() override;

    private:
	index_t instance_id_;
	lua_State* lua_state_;
        Object_var interpreter_; // Keeps refcount so that Lua interp
                                 // is not destroyed before the last
                                 // Callable
	static index_t current_instance_id_;
    };
}

#endif
