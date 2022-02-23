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
 
#ifndef H_OGF_BASIC_MODULES_MODMGR_H
#define H_OGF_BASIC_MODULES_MODMGR_H

#include <OGF/basic/common/common.h>
#include <OGF/basic/modules/module.h>
#include <geogram/basic/environment.h>
#include <string>
#include <vector>
#include <map>


/**
 * \file OGF/basic/modules/modmgr.h
 * \brief Management of dynamically loadable modules (plugins)
 */


/**
 * \brief Function pointer to a module initialization function.
 */
typedef void (*ModuleInitFunc)() ;

/**
 * \brief Function pointer to a module termination function.
 */
typedef void (*ModuleTerminateFunc)() ;

//____________________________________________________________________________

namespace OGF {
    
    /**
     * \brief Manages dynamically loadable modules. 
     */
    class BASIC_API ModuleManager : public Environment {
    public:

	/**
	 * \brief A generic function pointer.
	 */
	typedef void (*function_ptr)();

        /**
         * \brief Initializes the ModuleManager system.
         * \details Called once when Graphite starts.
         */
        static void initialize() ;

        /**
         * \brief Terminates the ModuleManager system.
         * \details Called once when Graphite exits.
         */
        static void terminate() ;

        /**
         * \brief Terminates the dynamic modules.
         * \details The dynamic modules are those that are loaded on-demand
         *   by Graphite. This function is called once when Graphite exits.
         */
        static void terminate_dynamic_modules() ;

        /**
         * \brief Gets a pointer to the ModuleManager instance.
         * \return The pointer to the instance of the ModuleManager.
         */
        static ModuleManager* instance() ;

        /**
         * \brief Loads a dynamic module. 
         * \details Searches a dll that matches the module name in the
         *  OGF path, managed by the FileManager.
         * \param[in] module_name the name of the module, as displayed 
         *  to the user (without "lib" prefix, without ".dll" or ".so"
         *  extension)
         * \param[in] quiet if true, status messages are displayed 
         * \retval true if the module was successfully loaded
         * \retval false otherwise
         * \see FileManager
         */
        bool load_module(const std::string& module_name, bool quiet = false) ;


        /**
         * \brief Declares a Module object to the ModuleManager.
         * \details Ownership of the module object is transfered
         *  to the module manager.
         * \param[in] module_name name of the module
         * \param[in] module a pointer to the module object
         * \retval true if the module could be successfully bound
         * \retval false otherwise (e.g. if a module with the same
         *  name was already bound)
         */
        bool bind_module(
            const std::string& module_name, Module* module
        ) ;

        /** 
         * \brief Removes a Module object from the ModuleManager.
         * \param[in] module_name name of the module.
         * \retval true if the module could be successfully unbound
         * \retval false otherwise (e.g. if no module of that name
         *  was found in the ModuleManager)
         */
        bool unbind_module(const std::string& module_name) ;

        /** 
         * \brief Retreives a Module object by name.
         * \param[in] module_name name of the module
         * \return the module object associated with \p module_name
         *  if it exists, nil otherwise
         */
        Module* resolve_module(const std::string& module_name) ;

        /**
         * \brief overloads Environment::get_local_value()
         * \details Defines the "loaded_modules" and 
         *  "loaded_dynamic_modules" variables.
         * \param[in] name name of the variable to be queried
         * \param[out] value value of the variable
         * \retval true if a variable with \p name was found
         * \retval false otherwise
         */
        virtual bool get_local_value(
            const std::string& name, std::string& value
        ) const;

        /**
         * \brief overloads Environment::set_local_value()
         * \details Defines no additional variable ("loaded_modules"
         *  and "loaded_dynamic_modules" are readonly).
         * \param[in] name name of the variable to be set
         * \param[in] value new value for the variable
         * \retval true if a variable with \p name could be set to \p value
         * \retval false otherwise
         */
        virtual bool set_local_value(
            const std::string& name, const std::string& value
        );

	/**
	 * \brief Finds a pointer to a function by its name.
	 * \details Searches all the functions currently loaded in all the
	 *  dynamic libraries used by the current process.
	 * \param[in] name the name of the function.
	 * \return a pointer to the function or nullptr if no function
	 *  was found.
	 */
	static function_ptr resolve_function(const std::string& name);

	/**
	 * \brief Finds a pointer to a symbol by its name.
	 * \details Searches all the functions currently loaded in all the
	 *  dynamic libraries used by the current process.
	 * \param[in] name the name of the symbol.
	 * \return a pointer to the symbol or nullptr if no symbol
	 *  was found.
	 */
	static void* resolve_symbol(const std::string& name);


	/**
	 * \brief Adds a path where dynamic libraries can be loaded.
	 * \details Under Windows, adds the path to the PATH environment
	 *  variable. Under Unixes, adds the path to the LD_LIBRARY_PATH
	 *  environment variable.
	 * \param[in] path the path to be added.
	 */
	static void append_dynamic_libraries_path(const std::string& path);
	
    protected:
        /**
         * \brief Forbids construction by client code.
         */
        ModuleManager() ;
        
        /**
         * \brief Forbids destruction by client code.
         */
        ~ModuleManager() ;

        /**
         * \brief Terminates all the modules registered in the system.
         */
        void do_terminate_modules() ;
        
    private:
        std::vector<ModuleTerminateFunc> to_terminate_ ;
        std::vector<void*> module_handles_ ;
        std::map<std::string, Module_var> modules_ ;
        static ModuleManager* instance_ ;
    } ;

//____________________________________________________________________________

}

#endif
