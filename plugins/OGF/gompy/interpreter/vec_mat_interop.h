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

#ifndef H_OGF_GOM_PYTHON_VEC_MAT_INTEROP_H
#define H_OGF_GOM_PYTHON_VEC_MAT_INTEROP_H

#include <OGF/gompy/common/common.h>
#include <OGF/gompy/interpreter/python.h>

namespace OGF {

    class Any;
    class MetaType;

    /**
     * \brief Converts a Python object into a Graphite vec or mat type
     * \details Works with vec2, vec3, vec4 of doubles and integers, and
     *   with mat4 of doubles.
     * \param[in] obj a pointer to the Python object
     * \param[out] result the result, as an Any
     * \param[in] mtype a pointer to the expected meta-type
     * \retval true if conversion was successful
     * \retval false otherwise
     */
    bool python_to_graphite_mat_vec(PyObject* obj, Any& result, MetaType* mtype);

}

#endif
