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

#ifndef H_OGF_GOM_PYTHON_INTEROP_H
#define H_OGF_GOM_PYTHON_INTEROP_H

#include <OGF/gompy/common/common.h>
#include <OGF/gompy/interpreter/python.h>
#include <OGF/gom/types/any.h>

/**
 * \file OGF/gom_python/interpreter/vec_mat_interop.h
 * \brief Functions to exchange objects between Python and Graphite
 */

namespace OGF {

    class ArgList;
    class MetaMethod;

    namespace GOMPY {
	/**
	 * \brief Converts a Python object to a string.
	 * \param[in] obj a pointer to the Python object.
	 * \return a string representation of the object.
	 */
	std::string python_to_string(PyObject* obj);

	/**
	 * \brief Converts a string to a Python string.
	 * \param[in] s the string to be converted.
	 * \return the string a s a Python object.
	 */
	PyObject* string_to_python(const std::string& s);

	/**
	 * \brief Converts a Python object to C++.
	 * \param[in] obj a pointer to the Python object.
	 * \param[in] mtype an optional MetaType. If unspecified, uses a
	 *   default conversion.
	 * \return the converted object in an Any.
	 */
	Any python_to_graphite(PyObject* obj, MetaType* mtype = nullptr);

	/**
	 * \brief Converts a C++ object to Python.
	 * \param[in] value the C++ object stored in an Any.
	 * \param[in] mtype an optional MetaType. If unspecified, uses the
	 *  type of the Any.
	 * \return the object converted to a Python object.
	 */
	PyObject* graphite_to_python(
	    const Any& value, MetaType* mtype = nullptr
	);

	/**
	 * \brief Converts Python arguments to a GOM ArgList.
	 * \param[in] args the arguments.
	 * \param[in] keywords the optional arguments as name-value pairs.
	 * \param[out] gom_args the constructed GOM ArgList.
	 * \param[in] mmethod an optional pointer to the MetaMethod
	 *  that will receive the arguments. It is needed if keywords
	 *  are not specified.
	 */
	void python_tographiteargs(
	    PyObject* args, PyObject* keywords,
	    ArgList& gom_args, MetaMethod* mmethod = nullptr
	);
    }

}

#endif
