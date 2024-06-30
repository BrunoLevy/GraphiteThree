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

#ifndef H_OGF_GOM_REFLECTION_META_ARG_H
#define H_OGF_GOM_REFLECTION_META_ARG_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_type.h>

#include <string>
#include <vector>

/**
 * \file OGF/gom/reflection/meta_arg.h
 * \brief Meta-information attached to arguments.
 */

namespace OGF {

    class MetaType ;

//______________________________________________________________

    /**
     * \brief The representation of the arguments in the Meta repository.
     */
    class GOM_API MetaArg : public CustomAttributes {
    public:

        /**
         * \brief Constructs a new MetaArg.
         * \param[in] name name of the argument
         * \param[in] type_name C++ type name of the argument
         */
	MetaArg(const std::string& name, const std::string& type_name) ;

        /**
         * \brief Constructs a new MetaArg.
         * \param[in] name name of the argument
         * \param[in] type pointer to the MetaType of the argument
         */
        MetaArg(const std::string& name, MetaType* type) ;

        /**
         * \brief Gets the name
         * \return the name of the argument
         */
        const std::string& name() const {
            return name_ ;
        } 

        /**
         * \brief Gets the type name name
         * \return the C++ type name of the argument
         */
        const std::string& type_name() const {
            return type_name_ ;
        }

        /**
         * \brief Gets the type 
         * \return a pointer to the MetaType of the argument
         */
        MetaType* type() const;

        /**
         * \brief Tests whether the argument has a default value
         * \retval true if the argument has a default value
         * \retval false otherwise
         */
        bool has_default_value() const {
            return !default_value_.is_null();
        }

        /**
         * \brief Gets the default value of an argument.
         * \return a const reference to an Any with the
         *  default value of the argument
         */
        const Any& default_value() const {
            return default_value_ ;
        }

        /**
         * \brief Gets the default value of an argument.
         * \return a modifiable reference to an Any with the
         *  default value of the argument
         */
	Any& default_value() {
	    return default_value_;
	}
        
    private:
        std::string name_ ;
        std::string type_name_ ;
	Any default_value_;
    } ;

    /**
     * \brief Meta representation of a list of arguments.
     */
    typedef std::vector<MetaArg> MetaArgList ;
//______________________________________________________________

}
#endif 

