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

#ifndef H_OGF_GOM_PYTHON_PY_GRAPHITE_OBJECT_H
#define H_OGF_GOM_PYTHON_PY_GRAPHITE_OBJECT_H

#include <OGF/gompy/common/common.h>
#include <OGF/gompy/interpreter/python.h>

namespace OGF {

    namespace GOMPY {

	/**
	 * \brief Tests whether a Python object is a Graphite object.
	 * \param[in] obj a pointer to the object.
	 * \retval true if the object is a Graphite object.
	 * \retval false otherwise.
	 */
	bool PyGraphite_Check(PyObject* obj);

	/**
	 * \brief Gets the Graphite object stored in a Python object
	 * \pre PyGraphite_Check(obj)
	 * \return a pointer to the Graphite object
	 */
	Object* PyGraphite_GetObject(PyObject* obj);

	/**
	 * \brief Creates a new Python wrapper around a Graphite object.
	 * \param[in] object a pointer to the object to be wrapped.
	 * \param[in] managed true if reference counting is enabled (default),
	 *  false otherwise. Reference counting is disabled for the interpreter
	 *  itself, else it creates a circular reference.
	 */
	PyObject* PyGraphiteObject_New(Object* object, bool managed=true);
    }
}

#endif
