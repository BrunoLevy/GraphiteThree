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

#ifndef H_OGF_GOM_PYTHON_PYTHON_CALLABLE_H
#define H_OGF_GOM_PYTHON_PYTHON_CALLABLE_H

#include <OGF/gompy/common/common.h>
#include <OGF/gom/types/callable.h>

/**
 * \file OGF/gom_python/interpreter/gom_python_callable.h
 * \brief Python callables exported to Graphite.
 */

// Python object forward declaration.
// We cannot forward-declare PyObject, and we cannot include Python headers,
// because the gom compiler won't parse them, so we forward-declare the internal
// struct name for PyObject.
struct _object;

namespace OGF {

    /*****************************************************************/

    /**
     * \brief GOM wrapper around a Python function.
     */
    gom_class gompy_API PythonCallable : public Callable {
    public:
        /**
         * \brief PythonCallable constructor.
         */
        PythonCallable(struct _object* impl);

        /**
         * \copydoc Callable::invoke()
         */
         bool invoke(const ArgList& args, Any& ret_val) override;

	/**
	 * \brief PythonCallable destructor.
	 */
         ~PythonCallable() override;

    private:
	struct _object* impl_;
    };

    /*****************************************************************/

}

#endif
