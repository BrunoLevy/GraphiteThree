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

#ifndef H_OGF_GOM_PYTHON_PY_GRAPHITE_ITERATOR_H
#define H_OGF_GOM_PYTHON_PY_GRAPHITE_ITERATOR_H

#include <OGF/gompy/common/common.h>
#include <OGF/gompy/interpreter/python.h>

/**
 * \file OGF/gom_python/interpreter/py_graphite_iterator.h
 * \brief Python iterator over a Graphite object seen as a collection
 */

namespace OGF {

    class Object;

    namespace GOMPY {

	/**
	 * \brief Function to initialize graphite_IteratorType
	 */
	void init_graphite_IteratorType();

	extern PyTypeObject graphite_IteratorType;

	/**
	 * \brief Creates a new iterator over a Graphite object.
	 * \param[in] object a pointer to the graphite object.
	 * \details Returns a Python iterator
	 */
	PyObject* PyGraphiteIterator_New(Object* object);

    }
}


#endif
