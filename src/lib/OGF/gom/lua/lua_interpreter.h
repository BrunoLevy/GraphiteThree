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

#ifndef H_OGF_GOM_LUA_INTERPRETER_H
#define H_OGF_GOM_LUA_INTERPRETER_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/gom/types/callable.h>

struct lua_State;

/**
 * \file OGF/gom/lua/lua_interpreter.h
 * \brief The main class that interfaces the Python interpreter with Graphite.
 */

namespace OGF {

    /*****************************************************************/

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
	static index_t current_instance_id_;
    };
    
    
    /*****************************************************************/
    
    /**
     * \brief The main class that interfaces the Python interpreter 
     *  with Graphite.
     */
    gom_class GOM_API LuaInterpreter : public Interpreter {
    public:
        /**
         * \brief LuaInterpreter constructor.
         */
        LuaInterpreter();
        
        /**
         * \brief LuaInterpreter destructor.
         */
        ~LuaInterpreter() override;

	/**
	 * \copydoc Interpreter::reset()
	 */
	void reset() override;
	
        /**
         * \copydoc Interpreter::execute()
         */
        bool execute(
            const std::string& command, bool save_in_history, bool log
        ) override;

        /**
         * \copydoc Interpreter::execute_file()
         */
        bool execute_file(const std::string& file_name) override;

	/**
	 * \copydoc Interpreter::resolve()
	 */
	Any resolve(const std::string& id, bool quiet=true) const override;

	/**
	 * \copydoc Interpreter::bind()
	 */
	void bind(const std::string& id, const Any& value) override;
	
	/**
	 * \copydoc Interpreter::eval()
	 */
	Any eval(
	    const std::string& expression, bool quiet=true
	) const override;

	/**
	 * \brief Gets the LUA state.
	 * \return a pointer to the LUA state.
	 */
	lua_State* lua_state() {
	    return lua_state_;
	}

	/**
	 * \brief Displays a Lua error message.
	 * \details Removes the repetition of the source-code between 
	 *  square brackets which is of little use to tue user.
	 * \param[in] msg the error message to display.
	 */
	static void display_error_message(const std::string& msg);

	/**
	 * \brief Lists the variable names in this Scope.
	 * \param[out] names a vector of strings with the names 
	 *  of the variables.
	 */
	void list_names(std::vector<std::string>& names) const override;
	
    protected:

	/**
	 * \brief Adjusts LUA state to recover from errors.
	 * \details Base class implementation does nothing. It can 
	 *  be overriden to recover from errors. For instance, it can
	 *  call adjust_lua_glup_state() if GLUP bindings are 
	 *  activated.
	 */
	virtual void adjust_lua_state();
	
	/**
	 * \copydoc Interpreter::get_keys()
	 */
	 void get_keys(
	     const std::string& context, std::vector<std::string>& keys
	 ) override;

	 /**
	  * \copydoc Interpreter::name_value_pair_call()
	  */
	 std::string name_value_pair_call(
	     const std::string& args
	 ) const override;

	 
    private:
	lua_State* lua_state_;	
    }; 

} 

#endif 
