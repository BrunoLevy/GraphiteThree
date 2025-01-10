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

#ifndef H_OGF_BASIC_MODULES_MODULE_H
#define H_OGF_BASIC_MODULES_MODULE_H

/**
 * \file OGF/basic/modules/module.h
 * \brief Class to represent information about a Graphite/// module
 *  (i.e. a plugin).
 */

#include <OGF/basic/common/common.h>
#include <string>

//____________________________________________________________________________

namespace OGF {

    /**
     * \brief Represents information associated
     *  with a module (i.e. a plugin).
     */
    class BASIC_API Module : public Counted {

    public:
        /**
         * \brief Module constructor.
         */
        Module();

	/**
	 * \brief Module destructor.
	 */
	~Module() override;

        /**
         * \brief Gets the name of the module.
         * \return a const reference to the name
         *  of the module.
         */
        const std::string& name() {
            return name_;
        }

        /**
         * \brief Sets the name of the module.
         * \param[in] name_in the name of the module
         * \details This function is not meant to be
         *  used by client code. It is called automatically
         *  in the initializer of the module.
         */
        void set_name(const std::string& name_in) {
            name_ = name_in;
        }

        /**
         * \brief Gets the vendor of this module
         * \return a string with the name of the vendor of
         *  this module (usually "OGF" for
         *  "Open Graphics Foundation")
         */
        const std::string& vendor() {
            return vendor_;
        }

        /**
         * \brief Sets the vendor of the module.
         * \param[in] vendor_in the name of the module's vendor
         * \details This function is not meant to be
         *  used by client code. It is called automatically
         *  in the initializer of the module.
         */
        void set_vendor(const std::string& vendor_in) {
            vendor_ = vendor_in;
        }

        /**
         * \brief Gets the version string.
         * \return a string that encodes the version of this module.
         */
        const std::string& version() {
            return version_;
        }

        /**
         * \brief Sets the version of the module.
         * \param[in] version_in a string that represents
         *  the version of the module
         * \details This function is not meant to be
         *  used by client code. It is called automatically
         *  in the initializer of the module.
         */
        void set_version(const std::string& version_in) {
            version_ = version_in;
        }

        /**
         * \brief Tests whether this module is dynamic.
         * \details Dynamic modules are those that are loaded
         *  on-demand by Graphite, i.e. those that are not
         *  linked with Graphite.
         */
        bool is_dynamic() const {
            return is_dynamic_;
        }

        /**
         * \brief Sets the is_dynamic flag of this module
         * \param[in] b the is_dynamic flag
         * \details This function is not meant to be
         *  used by client code. It is called automatically
         *  in the initializer of the module.
         * \see is_dynamic
         */
        void set_is_dynamic(bool b) {
            is_dynamic_ = b;
        }

        /**
         * \brief Tests whether this module belongs to the Graphite
         *  base system.
         * \retval true if this module is a system module
         * \retval false otherwise
         */
        bool is_system() const {
            return is_system_;
        }

        /**
         * \brief Sets the is_system flag of this module
         * \param[in] x the is_system flag
         * \details This function is not meant to be
         *  used by client code. It is called automatically
         *  in the initializer of the module.
         * \see is_system
         */
        void set_is_system(bool x) {
            is_system_ = x;
        }

        /**
         * \brief Gets the information string.
         * \return a string that gives a short description
         *  of the functionalities implemented in a plugin.
         */
        const std::string& info() {
            return info_;
        }

        /**
         * \brief Sets the information string associated
         *  with this module
         * \param[in] info_in the information string
         * \details This function is not meant to be
         *  used by client code. It is called automatically
         *  in the initializer of the module.
         */
        void set_info(const std::string& info_in) {
            info_ = info_in;
        }

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
        static bool bind_module(
            const std::string& module_name, Module* module
        );

        /**
         * \brief Removes a Module object from the ModuleManager.
         * \param[in] module_name name of the module.
         * \retval true if the module could be successfully unbound
         * \retval false otherwise (e.g. if no module of that name
         *  was found in the ModuleManager)
         */
        static bool unbind_module(const std::string& module_name);

        /**
         * \brief Retreives a Module object by name.
         * \param[in] module_name name of the module
         * \return the module object associated with \p module_name
         *  if it exists, nil otherwise
         */
        static Module* resolve_module(const std::string& module_name);

    private:
        std::string name_;
        std::string vendor_;
        std::string version_;
        bool is_dynamic_;
        bool is_system_;
        std::string info_;
    };

    /**
     * \brief An automatic reference-counted pointer to a Module object.
     */
    typedef SmartPointer<Module> Module_var;

//____________________________________________________________________________

}

#endif
