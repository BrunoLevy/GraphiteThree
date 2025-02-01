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

#ifndef H_OGF_GOM_PYTHON_NL_VECTOR_INTEROP_H
#define H_OGF_GOM_PYTHON_NL_VECTOR_INTEROP_H

#include <OGF/gompy/common/common.h>
#include <OGF/gompy/interpreter/python.h>
#include <OGF/scene_graph/NL/vector.h>

/**
 * \file OGF/gom_python/interpreter/vec_mat_interop.h
 * \brief Functions to exchange NL::Vector objects between Python and Graphite
 */

namespace OGF {

    class Any;
    class MetaType;

    namespace GOMPY {

	/**
	 * \brief Converts a Python object to a NL Vector
	 * \param[in] obj a pointer to the Python object
	 * \param[out] result the NL vector stored in a Any
	 * \param[in] mtype the expected meta-type:
	 *    ogf_meta<::OGF::NL::Vector*>::type()
	 * \retval true if conversion was successful, that is, Python object
	 *   supports array interface and mttype corresponds to NL::Vector*
	 *   meta-type
	 * \retval false otherwise
	 */
	bool python_to_nl_vector(PyObject* obj, Any& result, MetaType* mtype);

	/**
	 * \brief Creates an array interface to a Graphite NL::Vector
	 * \param[in] vector a pointer to the NL::Vector
	 * \return a PyCapsule that contains the PyArrayInterface
	 * \details Used for NumPy interop.
	 */
	PyObject* create_array_interface(NL::Vector* vector);

	/**
	 * \brief Deletes a PyArrayInterface encapsulated in a PyCapsule.
	 * \param[in] capsule a pointer to
	 *  the PyCapsule to be deleted.
	 * \details Used for NumPy interop.
	 */
	void delete_array_interface(PyObject* capsule);
    }
}

#endif
