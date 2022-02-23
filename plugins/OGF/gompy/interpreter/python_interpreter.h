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

#ifndef H_OGF_GOM_PYTHON_INTERPRETER_H
#define H_OGF_GOM_PYTHON_INTERPRETER_H

#include <OGF/gompy/common/common.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/gom/types/callable.h>

/**
 * \file OGF/gom_python/interpreter/python_interpreter.h
 * \brief The main class that interfaces the Python interpreter with Graphite.
 */

struct _object;

namespace OGF {

    /*****************************************************************/

    /**
     * \brief GOM wrapper around a Python function.
     */
    gom_class gompy_API PythonCallable : public Callable {
    public:
        /**
         * \brief PythonCallable constructor.
         */
        PythonCallable(struct _object* impl);

        /**
         * \copydoc Callable::invoke()
         */
         bool invoke(const ArgList& args, Any& ret_val) override;

	/**
	 * \brief PythonCallable destructor.
	 */
         ~PythonCallable() override;
	
    private:
	struct _object* impl_;
    };
    
    
    /*****************************************************************/
    
    /**
     * \brief The main class that interfaces the Python interpreter 
     *  with Graphite.
     */
    gom_class gompy_API PythonInterpreter : public Interpreter {
    public:
        /**
         * \brief PythonInterpreter constructor.
         */
        PythonInterpreter();
        
        /**
         * \brief PythonInterpreter destructor.
         */
        ~PythonInterpreter() override;

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
	 * \copydoc Interpreter::bind()
	 */
         void bind(const std::string& id, const Any& value) override;
	
	/**
	 * \copydoc Interpreter::resolve()
	 */
         Any resolve(const std::string& id, bool quiet=true) const override;

	/**
	 * \copydoc Interpreter::eval()
	 */
         Any eval(
	    const std::string& expression, bool quiet=true
	) const override;

	/**
	 * \brief Displays a Python error message.
	 * \details Removes the repetition of the source-code between 
	 *  square brackets which is of little use to tue user.
	 * \param[in] msg the error message to display.
	 */
	static void display_error_message(const std::string& msg);

	/**
	 * \copydoc Interpreter::list_names()
	 */
        void list_names(std::vector<std::string>& names) const override;
	
      protected:
	/**
	 * \copydoc Interpreter::get_keys()
	 */
	 void get_keys(
	     const std::string& context, std::vector<std::string>& keys
	 ) override;

      private:
	struct _object* main_module_;
	bool use_embedded_interpreter_;
    }; 

} 


#endif 
