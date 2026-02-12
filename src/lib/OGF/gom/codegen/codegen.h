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


#ifndef H_GOM_CODEGEN_CODEGEN_H
#define H_GOM_CODEGEN_CODEGEN_H

#include <OGF/gom/common/common.h>

#include <iostream>
#include <string>
#include <map>
#include <set>

/**
 * \file OGF/gom/codegen/codegen.h
 * \brief Code generator for GOM meta information.
 */

namespace OGF {

    class MetaType;
    class MetaClass;
    class MetaBuiltinType;
    class MetaEnum;
    class MetaMethod;
    class MetaSignal;
    class MetaConstructor;
    class MetaInformation;
    class CustomAttributes;
    class MetaArg;
    class Any;

    /**
     * \brief Generates C++ code to create the GOM meta information.
     * \details The GOM parser populates the Meta reflection API with
     *  description of C++ types (MetaClass, MetaEnum ...). GomCodeGenerator
     *  generates code to recreate this description, so that Graphite does
     *  not need to parse header files at startup. In addition, GomCodeGenerator
     *  generates method adapters and factories to invoke methods and create
     *  objects from Python scripts.
     */
    class GOM_API GomCodeGenerator {
    public:

        /**
         * \brief GomCodeGenerator constructor.
         */
        GomCodeGenerator();


        /**
         * \brief Generates C++ code to create the Meta information
         *  and adapters.
         * \details Dependencies (e.g., superclasses) are also generated,
         *  provided that they belong to package \p package_name.
         * \param[out] out a reference to the stream that will
         *  receive the generated C++ code
         * \param[in] classes the list of classes to generate,
         *  specified as a vector of pointers to MetaClass objects.
         * \param[in] package_name the name of the package
         */
        void generate(
            std::ostream& out, std::vector<MetaClass*> classes,
            const std::string& package_name
        );

    protected:

        /**
         * \brief Generates C++ code that creates the meta information
         *  associated with a class.
         * \details C++ code is generated in the stream returned by out().
         *  The function also generates dependencies (superclasses).
         * \param[in] type pointer to the MetaClass
         * \see out() generate_class()
         */
        void generate(MetaClass* type);

        /**
         * \brief Generates C++ code that creates the meta
         *  information associated with a builtin type.
         * \details C++ code is generated in the stream returned by out().
         * \param[in] mbuiltin pointer to the MetaBuiltinType
         * \see out()
         */
        void generate_builtin(MetaBuiltinType* mbuiltin);

        /**
         * \brief Generates C++ code that creates the meta information
         *  associated with an enum.
         * \details C++ code is generated in the stream returned by out().
         * \param[in] menum pointer to the MetaEnum
         * \see out()
         */
        void generate_enum(MetaEnum* menum);

        /**
         * \brief Generates C++ code that creates the meta information
         *  associated with a class.
         * \details C++ code is generated in the stream returned by out().
         *  The function only generates this MetaClass (and does not take
         *  care of dependencies).
         * \param[in] mclass pointer to the MetaClass
         * \see out() generate(MetaClass*)
         */
        void generate_class(MetaClass* mclass);

        /**
         * \brief Generates a method adapter.
         * \details C++ code is generated in the stream returned by out().
         * \param[in] method a pointer to the MetaMethod
         */
        void generate_method_adapter(MetaMethod* method);

        /**
         * \brief Generates a method adapter.
         * \details This version is used for the special case where the
         *   method takes a ArgList as an argument.
         *  C++ code is generated in the stream returned by out().
         * \param[in] method a pointer to the MetaMethod
         * \see generate_method_adapter()
         */
        void generate_method_adapter_arglist(MetaMethod* method);

        /**
         * \brief Generates a signal adapter.
         * \details C++ code is generated in the stream returned by out().
         * \param[in] signal a pointer to the MetaSignal
         * \see generate_method_adapter()
         */
        void generate_signal_adapter(MetaSignal* signal);

        /**
         * \brief Generates a factory from a MetaConstructor.
         * \details C++ code is generated in the stream returned by out().
         * \param[in] constructor a pointer to the MetaConstructor
         * \see generate_method_adapter()
         */
        void generate_factory(MetaConstructor* constructor);

        /**
         * \brief Generates C++ code that creates the CustomAttributes
         *  associated with a language construct.
         * \param[in] info pointer to an object that inherits CustomAttributes
         * \param[in] variable_name the name of the C++ variable in the
         *  generated code where custom attributes should be copied
         * \param[in] is_pointer true if variable_name is a pointer (to a class
         *  that inherits CustomAttributes), false if variable_name is a
         *  reference.
         */
        void generate_attributes(
            const CustomAttributes* info, const std::string& variable_name,
            bool is_pointer=true
        );

        /**
         * \brief Adds double quotes to a string.
         * \param[in] s a const reference to the input string
         * \return the string \p s with double quotes
         */
        std::string stringify(const std::string& s);

        /**
         * \brief Generates the code that computes an object
         *   that represents a default value.
         * \details The default value can be either an immediate
         *   value or a constructed object, and the behavior is
         *   different in both cases.
         * \param[in] marg a const reference to the input MetaArg
         * \retval the string \p s with double quotes if default
         *   value is a string literal. It is the case if MetaArg
	 *   is of type std::string or derived from a Name class.
	 *   Since we cannot test for the latter, we guess it is the
	 *   case if typename constains "Name" as a substring.
         * \retval the constant that corresponds to the default value
	 *   otherwise
         */
        std::string stringify_default_value(const MetaArg* marg);

        /**
         * \brief Replaces all colons (":") with underscores ("_") in a string.
         * \param[in] s a const reference to the input string
         * \return \p s with all colons replaced with underscores
         */
        std::string colons_to_underscores(const std::string& s);

        /**
         * \brief Generates a C++ name for a method adapter from
         *  a MetaMethod.
         * \param[in] method a pointer to the MetaMethod
         * \return a valid and unique C++ name for a method adapter
         */
        std::string method_adapter_name(MetaMethod* method);

        /**
         * \brief Generates a C++ name for a factory from
         *  a MetaMethod.
         * \param[in] constructor a pointer to the MetaConstructor
         * \return a valid and unique C++ name for a factory
         */
        std::string factory_name(MetaConstructor* constructor);

    protected:

        /**
         * \brief Gets a reference to the output stream, where
         *  the generated C++ code is sent.
         * \return a reference to the output stream
         */
        std::ostream& out() {
            return *out_;
        }

        /**
         * \brief Tests whether objects of a given type should
         *  be passed by value or by reference.
         * \param[in] type_name C++ type name, as a string
         * \retval true if objects of type \p type_name should be passed
         *   by value
         * \retvavl false if objects of type \p type_name should be
         *   passed by reference
         */
        bool pass_by_value(const std::string& type_name);

    private:
        std::ostream* out_;
        std::set<std::string> pass_by_value_;
        std::set<MetaClass*> to_generate_;
        std::vector<MetaClass*> sorted_;
        std::string package_name_;
    };

}

#endif
