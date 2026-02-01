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

#ifndef H_OGF_BASIC_TYPES_ARG_LIST_H
#define H_OGF_BASIC_TYPES_ARG_LIST_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/types/any.h>
#include <geogram/basic/string.h>
#include <geogram/basic/memory.h>
#include <iostream>
#include <type_traits>

/**
 * \file OGF/gom/types/arg_list.h
 * \brief Generic arguments and argument lists.
 */

namespace OGF {

    /**
     * \brief Base class for all Names in Graphite (GrobName ...).
     * \details Used by the mechanism that transforms an Object* to its
     *  name for functions that take names.
     */
    class NameBase {
    };

    /**
     * \brief Represents a list of name-value pairs.
     */
    class GOM_API ArgList {

    public:

        /**
         * \brief ArgList constructor.
         */
        ArgList() {
	    argval_.reserve(4);
	    argname_.reserve(4);
        }

        /**
         * \brief ArgList copy-constructor.
         * \param[in] rhs a const reference to the ArgList to be copied
         */
        ArgList(const ArgList& rhs) :
           argval_(rhs.argval_), argname_(rhs.argname_) {
        }

        /**
         * \brief ArgList assignment operator
         * \param[in] rhs a const reference to the ArgList to be copied
         */
        ArgList& operator=(const ArgList& rhs) {
	    argval_ = rhs.argval_;
	    argname_ = rhs.argname_;
            return *this;
        }

        /**
         * \brief Gets the number of arguments.
         * \return the number of arguments in this ArgList
         */
        index_t nb_args() const {
            return argname_.size();
        }

        /**
         * \brief Tests whether this ArgList has unnamed args
         * \retval true if this ArgList has unnamed args
         * \retval false otherwise
         */
        bool has_unnamed_args() const {
            if(nb_args() == 0) {
                return false;
            }
            for(index_t i=0; i<nb_args(); ++i) {
                if(ith_arg_name(i) != "arg#" + String::to_string(i)) {
                    return false;
                }
            }
            return true;
        }

        /**
         * \brief Tests whether an argument of a given name
         *  exists in this ArgList.
         * \param[in] name a const reference to the name of the
         *  argument
         * \retval true if this ArgList has an argument with name
         *  \p name
         * \retval false otherwise
         */
        bool has_arg(const std::string& name) const {
            return (find_arg_index(name) != NO_INDEX);
        }

        /**
         * \brief Finds argument index by name
         * \param[in] name a const reference to the name
         * \return the index of the argument with name \p name
         *  if it exists or NO_INDEX if there is no such
         *  argument
         */
        index_t find_arg_index(const std::string& name) const;

        /**
         * \brief Deletes an argument by index.
         * \param[in] index the index of the argument to be deleted
         * \pre \p i < nb_args()
         */
        void delete_ith_arg(index_t index) {
            geo_debug_assert(index < nb_args());
            argval_.erase(argval_.begin()+index);
	    argname_.erase(argname_.begin()+index);
        }

        /**
         * \brief Gets argument name by index.
         * \param[in] i the index of the argument
         * \return a const reference to the name of argument at
         *  index \p i
         * \pre \p i < nb_args()
         */
        const std::string& ith_arg_name(index_t i) const {
            geo_debug_assert(i < nb_args());
            return argname_[i];
        }

        /**
         * \brief Gets argument value by index.
         * \details If the stored argument is of type \p T, then it
         *  is retreived directly, else it is converted using a
         *  temporary string representation.
         * \param[in] i the index of the argument
         * \return the value of argument at
         *  index \p i
         * \tparam T the type of the argument to be retreived
         * \pre \p i < nb_args()
         */
        template <class T> T ith_arg_value(index_t i) const {
            geo_debug_assert(i < nb_args());
	    T result;
	    if(
                !argval_[i].get_value(result) &&
                !get_name(argval_[i], result)
            ) {
		arg_type_error(i, typeid(T).name());
	    }
	    return result;
        }

        /**
         * \brief Gets argument value by index,
	 *  stored as an Any.
         * \param[in] i the index of the argument
         * \return a const reference to the argument
	 *   value at index \p i, wrapped in an Any.
         * \pre \p i < nb_args()
         */
	const Any& ith_arg_value(index_t i) const {
            geo_debug_assert(i < nb_args());
	    return argval_[i];
	}

        /**
         * \brief Gets argument value by index,
	 *  stored as an Any.
         * \param[in] i the index of the argument
         * \return a modifiable reference to the argument
	 *  value at index \p i, wrapped in an Any.
         * \pre \p i < nb_args()
         */
	Any& ith_arg_value(index_t i) {
            geo_debug_assert(i < nb_args());
	    return argval_[i];
	}

        /**
         * \brief Gets argument value by name,
	 *  stored as an Any.
         * \param[in] name the name of the argument
         * \return a const reference to the argument
	 *  value at index \p i, wrapped in an Any.
         * \pre has_arg(name)
         */
	const Any& arg_value(const std::string& name) const {
	    index_t i = find_arg_index(name);
	    geo_debug_assert(i != NO_INDEX);
	    return argval_[i];
	}

        /**
         * \brief Gets argument value by name,
	 *  stored as an Any.
         * \param[in] name the name of the argument
         * \return a modifiable reference to the argument
	 *  value at index \p i, wrapped in an Any.
         */
	 Any& arg_value(const std::string& name) {
	    index_t i = find_arg_index(name);
	    geo_debug_assert(i != NO_INDEX);
	    return argval_[i];
	}


        /**
         * \brief Gets the type of an argument by index.
         * \param[in] i the index of the argument
         * \return a pointer to the type of the argument
         * \pre \p i < nb_args()
         */
        MetaType* ith_arg_type(index_t i) const {
            geo_debug_assert(i < nb_args());
	    return argval_[i].meta_type();
	}

        /**
         * \brief Creates an uninitialized argument.
         * \param[in] name a const reference to the name of the argument.
	 * \return a reference to the Any that will store the argument.
         * \pre !has_arg(name)
         */
        Any& create_arg(const std::string& name) {
            geo_debug_assert(!has_arg(name));
            argval_.push_back(Any());
	    argname_.push_back(name);
	    return *(argval_.rbegin());
        }

        /**
         * \brief Creates an uninitialized unnamed argument.
	 * \return a reference to the Any that will store the argument.
         * \pre nb_args() == 0 || has_unnamed_args()
         */
        Any& create_unnamed_arg() {
            geo_debug_assert(nb_args() == 0 || has_unnamed_args());
            return create_arg("arg#" + String::to_string(nb_args()));
        }

        /**
         * \brief Creates an argument.
         * \param[in] name a const reference to the name of the argument
         * \param[in] value a const reference to the value of the argument
         * \tparam T the type of the argument
         * \pre !has_arg(name)
         */
        template <class T> void create_arg(
            const std::string& name, const T& value
        ) {
            geo_debug_assert(!has_arg(name));
            argval_.push_back(Any());
	    argname_.push_back(name);
	    argval_.rbegin()->set_value(value);
        }

        /**
         * \brief Creates an argument from a string litteral.
         * \param[in] name a const reference to the name of the argument
         * \param[in] value a const pointer to a string
         * \pre !has_arg(name)
         */
        void create_arg(
            const std::string& name, const char* value
        ) {
            create_arg<std::string>(name, value);
        }

        /**
         * \brief Creates an argument from an Any.
         * \param[in] name a const reference to the name of the argument
         * \param[in] value an Any with the value
         * \pre !has_arg(name)
         */
        void create_arg(
            const std::string& name, const Any& value
        ) {
            geo_debug_assert(!has_arg(name));
            argval_.push_back(value);
	    argname_.push_back(name);
        }

        /**
         * \brief Sets an argument.
         * \details If the argument already exists, then its value
         *  is replaced with the new value, else a new argument
         *  is created.
         * \param[in] name a const reference to the name of the argument
         * \param[in] value a const reference to the value of the argument
         * \tparam T the type of the argument
         */
        template <class T> void set_arg(
            const std::string& name, const T& value
        ) {
	    index_t i = find_arg_index(name);
	    if(i == NO_INDEX) {
		create_arg(name, value);
	    } else {
		argval_[i].set_value(value);
	    }
        }

        /**
         * \brief Sets an argument from a string litteral.
         * \details If the argument already exists, then its value
         *  is replaced with the new value, else a new argument
         *  is created.
         * \param[in] name a const reference to the name of the argument
         * \param[in] value a const pointer to a string
         */
        void set_arg(const std::string& name, const char* value) {
            set_arg<std::string>(name, value);
        }


        /**
         * \brief Sets an argument from an Any.
         * \details If the argument already exists, then its value
         *  is replaced with the new value, else a new argument
         *  is created.
         * \param[in] name a const reference to the name of the argument
         * \param[in] value the value as an Any.
         */
        void set_arg(const std::string& name, const Any& value) {
	    index_t i = find_arg_index(name);
	    if(i == NO_INDEX) {
		create_arg(name, value);
	    } else {
		argval_[i] = value;
	    }
        }

        /**
         * \brief Sets an argument by index.
         * \details The name of the argument is kept.
         * \param[in] i the index of the argument
         * \param[in] value a const reference to the value of the argument
         * \tparam T the type of the argument
         */
        template<class T> void set_ith_arg(index_t i, const T& value) {
	    geo_debug_assert(i < nb_args());
	    argval_[i].set_value(value);
        }

        /**
         * \brief Sets an argument by index.
         * \details The name of the argument is kept.
         * \param[in] i the index of the argument
         * \param[in] value the value of the argument as a string litteral
         */
        void set_ith_arg(index_t i, const char* value) {
            set_ith_arg<std::string>(i, value);
        }

        /**
         * \brief Gets a string representation of an argument.
         * \param[in] name the name of the argument
         * \return the string representation of the argument
         * \pre has_arg(name)
         */
        std::string get_arg(const std::string& name) const {
	    index_t i = find_arg_index(name);
	    geo_debug_assert(i != NO_INDEX);
	    return argval_[i].as_string();
        }

        /**
         * \brief Gets an argument by name.
         * \details If the stored argument is of type \p T, then
         *  it is retreived directly, else it is converted using
         *  a temporary string representation.
         * \param[in] name the name of the argument
         * \return the value of the argument
         * \pre has_arg(name)
         * \tparam T the type of the argument
         */
        template <class T> T get_arg(const std::string& name) const {
	    index_t i = find_arg_index(name);
	    geo_debug_assert(i != NO_INDEX);
	    T result;
	    if(
                !argval_[i].get_value(result) &&
                !get_name(argval_[i], result)
            ) {
		arg_type_error(i, typeid(T).name());
	    }
	    return result;
        }

        /**
         * \brief Gets the type of an argument.
         * \param[in] name the name of the argument
         * \return a pointer to the type of the argument
         * \pre has_arg(name)
         */
        MetaType* get_arg_type(const std::string& name) const {
	    index_t i = find_arg_index(name);
	    geo_debug_assert(i != NO_INDEX);
	    return argval_[i].meta_type();
	}

        /**
         * \brief Removes all the arguments from this ArgList.
         */
        void clear() {
	    argval_.resize(0);
	    argname_.resize(0);
	}

        /**
         * \brief Appends all the arguments from an ArgList to this
         *  one.
         * \param[in] rhs a const reference to the ArgList to be appened
         * \param[in] overwrite if true, arguments in \p rhs that have
         *  the same names as arguments in this ArgList overwrite the
         *  previous values, else the previous values remain unchanged
         */
        void append(const ArgList& rhs, bool overwrite = true);

        /**
         * \brief Appends an argument from an ArgList to this one.
         * \param[in] rhs a const reference to the ArgList to be appened
         * \param[in] i the index of the argument to append
         * \param[in] overwrite if true, the argument in \p rhs that have
         *  the same name as the argument in this ArgList overwrites the
         *  previous values, else the previous value remain unchanged
         * \pre \p i < rhs.nb_args()
         */
        void append_ith_arg(
            const ArgList& rhs, index_t i, bool overwrite = true
        );

	void serialize(std::ostream& out) const;

	/**
	 * \brief Displays an error message for invalid argument type.
	 * \param[in] i the index of the concerned argument.
	 * \param[in] expected_typeid_name the typeid name that corresponds
	 *  to the expected type (that we did not have).
	 */
	void arg_type_error(
	    index_t i, const std::string& expected_typeid_name
	) const;


        /**
         * \brief Converts a pointer to Object into a string using its name
         *  attribute if target type is a xxxName
         * \details Used by the mechanism that lets one pass Object* to functions
         *  that take ObjectNames
         * \param[in] argval the stored value
         * \param[out] name the name to be read
         * \retval true if \p argval is a pointer to Object that has
         *  a name attribute and \p name's type is a xxxName
         * \retval false otherwise
         */
        template <class T> static bool get_name(const Any& argval, T& name) {
            if(!std::is_base_of<NameBase, T>::value) {
                return false;
            }
            Any name_prop;
            if(!get_object_name(argval, name_prop)) {
                return false;
            }
	    return name_prop.get_value(name);
        }

        /**
         * \brief Gets the name of an object
         * \details Used by the mechanism that lets one pass Object* to functions
         *  that take ObjectNames
         * \param[in] object an Any with a pointer to the object
         * \param[out] name an Any with the name of the object
         * \retval true if \p object has a pointer to an Object that has
         *  a name property
         * \retval false otherwise
         */
        static bool get_object_name(const Any& object, Any& name);


    private:
        vector<Any> argval_;
	vector<std::string> argname_;
    };


    /**
     * \brief Prints an ArgList into a stream.
     * \param[out] out the stream where the ArgList should be printed
     * \param[in] args a const reference to the ArgList to be printed
     */
    inline std::ostream& operator<<(std::ostream& out, const ArgList& args) {
	args.serialize(out);
	return out;
    }
}

#endif
