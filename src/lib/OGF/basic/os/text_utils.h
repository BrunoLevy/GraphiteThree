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


#ifndef H_OGF_BASIC_OS_TEXT_UTILS_H
#define H_OGF_BASIC_OS_TEXT_UTILS_H

#include <OGF/basic/common/common.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>

/**
 * \file OGF/basic/os/text_utils.h
 * \brief Utilities to manipulate text files.
 */

namespace OGF {
    
    namespace TextUtils {

        /**
         * \brief Manages a set of name-value or name-values pairs, used to
         *  manipulate text files.
         */
        class BASIC_API Environment {
        public:
            /**
             * \brief Tests whether a variable exists in this Environment.
             * \param[in] name name of the variable
             * \retval true if the variable exists
             * \retval false otherwise
             */
            bool has_variable(const std::string& name) const ;
            
            /**
             * \brief Gets the value of a variable.
             * \param[in] name name of the variable
             * \return the value of variable \p variable if it existed, or
             *  an empty string otherwise, then this creates the variable
             */
            std::string value(const std::string& name) const ;

            /**
             * \brief Gets the vector of values associated with a variable.
             * \details If the variable does not exists, throws an assertion
             *  failure.
             * \param[in] name name of the variable
             * \return a const reference to the vector of values associated with 
             *  variable \p name
             * \pre has_variable(name)
             */
            const std::vector<std::string>& values(const std::string& name) const ;

            /**
             * \brief Sets the value of a variable.
             * \details If the variable did not exist, it creates it. If it already
             *  existed, it overwrites its previous content. If value has the form
             *  "$var" and a variable named "var" exists in this Environment, then it
             *  is replaced with the set of values of variable "var".
             * \param[in] name name of the variable
             * \param[in] value new value of the variable
             */
            void set_value(const std::string& name, const std::string& value) ;

            /**
             * \brief Sets the values of a variable.
             * \details If the variable did not exist, it creates it. If it already
             *  existed, it overwrites its previous content. If one of the values has the form
             *  "$var" and a variable named "var" exists in this Environment, then it
             *  is replaced with the set of values of variable "var".
             * \param[in] name name of the variable
             * \param[in] values new vector of values to be associated with the variable
             */
            void set_values(const std::string& name, const std::vector<std::string>& values) ;

            /**
             * \brief Appends a value to the set of values associated with a variable.
             * \details If the variable did not exist, it creates it. If one of the 
             *  values has the form "$var" and a variable named "var" exists in this 
             *  Environment, then it is replaced with the set of values of variable "var".
             * \param[in] name name of the variable
             * \param[in] value value to be associated with the variable
             */
            void append_value(const std::string& name, const std::string& value) ;

            /**
             * \brief Appends a set of values to the set of values associated with a variable.
             * \details If the variable did not exist, it creates it. If one of the 
             *  values has the form "$var" and a variable named "var" exists in this 
             *  Environment, then it is replaced with the set of values of variable "var".
             * \param[in] name name of the variable
             * \param[in] values new vector of values to be associated with the variable
             */
            void append_values(const std::string& name, const std::vector<std::string>& values) ;

            /**
             * \brief Clears the values of a variable.
             * \details If the variable did not exist, it creates it with an empty value. If it
             *  already existed, it overwrites its contents with an empty value.
             * \param[in] name name of the variable
             */
            void clear_value(const std::string& name) ;

            /**
             * \brief Removes all the variables and their values.
             */
            void clear() ;

            /**
             * \brief Outputs the contents of this Enviroment to a stream.
             * \param[in,out] out the stream where the contents of this Environment
             *  should be output
             */
            void print(std::ostream& out) const ;

        private:
            typedef std::map< std::string, std::vector<std::string> > EnvMap ;        
            EnvMap data_ ;
        } ;


        /**
         * \brief Outputs the contents of an Enviroment to a stream.
         * \param[in,out] out the stream where the contents of the Environment
         *  should be output
         * \param[in] env a const reference to the Environment
         * \return the new state of the stream
         */
        inline std::ostream& operator<<(std::ostream& out, const Environment& env) {    
            env.print(out) ;
            return out ;
        }

        /**
         * \brief Reads an Environment from a file.
         * \param[in] file_name name of the file
         * \param[out] environment where to read the file
         */
        void BASIC_API read_environment_file(
            const std::string& file_name,
            Environment& environment
        ) ;

        /**
         * \brief Performs variables substitution in a stream
         * \details Whenever %varname% is encountered in the
         *  input stream, if a variable named "varname" exists
         *  in the environment, then the string is replaced with
         *  its contents, else it is kept as is.
         * \param[in] in the input stream
         * \param[out] out the output stream
         * \param[in] env a const reference to the environment
         */
        void BASIC_API find_and_replace(
            std::istream& in, std::ostream& out,
            const Environment& env
        ) ;

        /**
         * \brief Copies the content of an input stream to an
         *  output stream.
         * \param[in] in the input stream
         * \param[out] out the output stream
         */
        void BASIC_API concatenate(
            std::istream& in, std::ostream& out
        ) ;

        /**
         * \brief Tests whether a file contains an occurence
         *  of a string.
         * \param file_name name of the file
         * \param x the string
         * \retval true if file \p file_name contains at least
         *  an occurence of string \p x.
         * \retval false otherwise
         */
        bool BASIC_API file_contains_string(
            const std::string& file_name, const std::string& x
        ) ;
    }
}

#endif
