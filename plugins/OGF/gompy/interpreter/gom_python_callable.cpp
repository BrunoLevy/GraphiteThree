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

#include <OGF/gompy/interpreter/gom_python_callable.h>
#include <OGF/gompy/interpreter/interop.h>
#include <OGF/gompy/interpreter/python.h>
#include <geogram/basic/process.h>


namespace OGF {

    using namespace GOMPY;

    PythonCallable::PythonCallable(PyObject* impl) : impl_(impl) {
        Py_INCREF(impl);
        geo_assert(PyCallable_Check(impl_));
    }

    PythonCallable::~PythonCallable() {
        geo_assert(impl_ != nullptr);
        Py_DECREF(impl_);
        impl_ = nullptr;
    }

    bool PythonCallable::invoke(const ArgList& args_in, Any& ret_val) {
	// TODO: check number of parameters
	//   (Python: inspect.signature(func).parameters)
        // TODO: check reference counting, is this correct ?

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);

        PyObject* args = PyTuple_New(Py_ssize_t(args_in.nb_args()));
        Py_INCREF(args);
        PyObject* kw = PyDict_New();
        Py_INCREF(kw);
        for(unsigned int i=0; i<args_in.nb_args(); i++) {
            PyObject* name = string_to_python(args_in.ith_arg_name(i));
            PyObject* value = graphite_to_python(args_in.ith_arg_value(i));
            //  PyTuple_SetItem(args, i, value);
            // Does not work with this one, using the other one.
            PyTuple_SET_ITEM(args, i, value);
            Py_INCREF(value); // I think I need to do that
            PyDict_SetItem(kw, name, value);
            // I'm not sure whether I should incref on
            // name and value, it seems that it's not the
            // case (according to the doc)
        }

        geo_assert(impl_ != nullptr);
        geo_assert(PyCallable_Check(impl_));

        PyObject* result = PyObject_Call(impl_, args, nullptr);
            // Finally I'm not using kw...
	ret_val = python_to_graphite(result);
        Py_DECREF(args);
        Py_DECREF(kw);

	if(result == nullptr) {
	    PyErr_Print();
	}

	Py_XDECREF(result);

	Process::enable_FPE(FPE_bkp);

	return true;
    }

}
