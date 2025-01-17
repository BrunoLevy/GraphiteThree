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

#ifndef H_OGF_GOM_PYTHON_PY_GRAPHITE_METACLASS_H
#define H_OGF_GOM_PYTHON_PY_GRAPHITE_METACLASS_H

#include <OGF/gompy/common/common.h>
#include <OGF/gompy/interpreter/python.h>
#include <geogram/basic/numeric.h>

namespace OGF {

    class Object;

    namespace GOMPY {

	/**
	 * \brief A Python wrapper for Graphite MetaClasses.
	 */
	struct graphite_MetaClass {
	    PyTypeObject head;

	    /** \brief Pointer to the implementation. */
	    Object* object;

	    /** \brief Needed for metaclass to be seen as a python type,
		but I do not understand what it is. */
	    PyObject* weakrefs;

	    Numeric::uint32 magic;
	};

	constexpr Numeric::uint32 graphite_MetaClass_MAGIC = 0xbeeff00d;

	/**
	 * \brief Function to initialize graphite_MetaClassType
	 */
	void init_graphite_MetaClassType();

	/**
	 * \brief Class definition for Python wrapper
	 *  around Graphite MetaClass objects.
	 */
	extern PyTypeObject graphite_MetaClassType;

	/**
	 * \brief Tests whether a Python object is a Graphite MetaClass.
	 * \param[in] obj a pointer to the object.
	 * \retval true if the object is a Graphite MetaClass
	 * \retval false otherwise.
	 */
	bool PyGraphiteMetaClass_Check(PyObject* obj);

    }

}

#endif
