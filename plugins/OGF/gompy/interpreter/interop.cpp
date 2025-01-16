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

#include <OGF/gompy/interpreter/interop.h>
#include <OGF/gompy/interpreter/vec_mat_interop.h>
#include <OGF/gompy/interpreter/nl_vector_interop.h>
#include <OGF/gompy/interpreter/py_graphite_object.h>
#include <OGF/gompy/interpreter/gom_python_callable.h>
#include <OGF/gom/reflection/meta.h>

namespace OGF {

    namespace GOMPY {

	/**********************************************************************/

	std::string python_to_string(PyObject* obj) {
	    std::string result;
	    if(PyUnicode_Check(obj)) {
		Py_ssize_t size;
		const char* str = PyUnicode_AsUTF8AndSize(obj, &size);
		result = std::string(str, size_t(size));
	    } else if(PyGraphite_Check(obj)) {
		Object* graphite_obj = PyGraphite_GetObject(obj);
		ogf_convert_to_string(graphite_obj, result);
	    } else if(obj == Py_True) {
		result = "true";
	    } else if(obj == Py_False) {
		result = "false";
	    } else {
		PyObject* s = PyObject_Str(obj);
		Py_ssize_t size;
		const char* str = PyUnicode_AsUTF8AndSize(s, &size);
		// [TODO: check whether this is correct]
		result = std::string(str, size_t(size));
		Py_DECREF(s);
	    }
	    return result;
	}

	PyObject* string_to_python(const std::string& s) {
	    return PyUnicode_FromString(s.c_str()); // [TODO: is it correct ?]
	}

	/**********************************************************************/

	Any python_to_graphite(PyObject* obj, MetaType* mtype) {
	    Any result;
	    if(obj == nullptr) {
		return result;
	    }
	    if(python_to_graphite_mat_vec(obj, result, mtype)) {
		return result;
	    }
	    if(python_to_nl_vector(obj, result, mtype)) {
		return result;
	    }
	    if(PyGraphite_Check(obj)) {
		result.set_value(PyGraphite_GetObject(obj));
	    } else if(obj == Py_True) {
		result.set_value(true);
	    } else if(obj == Py_False) {
		result.set_value(false);
	    } else if(PyLong_Check(obj)) {
		result.set_value(index_t(PyLong_AsLong(obj)));
	    } else if(PyFloat_Check(obj)) {
		result.set_value(PyFloat_AsDouble(obj));
	    } else if(PyCallable_Check(obj)) {
		result.set_value(new PythonCallable(obj));
	    } else {
		result.set_value(python_to_string(obj));
	    }
	    return result;
	}

	/***********************************************************************/

	PyObject* graphite_to_python(const Any& arg, MetaType* mtype) {

	    if(mtype == nullptr && !arg.is_null()) {
		mtype = arg.meta_type();
	    }

	    if(mtype != nullptr && Any::is_pointer_type(mtype)) {
		MetaType* pmtype = Any::pointed_type(mtype);
		if(dynamic_cast<MetaClass*>(pmtype) != nullptr) {
		    Object* object = nullptr;
		    arg.get_value(object);
		    // if object pointer is null, return None
		    if(object == nullptr) {
			Py_INCREF(Py_None);
			return Py_None;
		    }
		    PyObject* result = PyGraphiteObject_New(object);
		    Py_INCREF(result);
		    return result;
		}
	    }

	    if(mtype == nullptr || mtype == ogf_meta<void>::type()) {
		Py_INCREF(Py_None);
		return Py_None;
	    }

	    if(mtype == ogf_meta<bool>::type()) {
		bool value;
		arg.get_value(value);
		if(value) {
		    Py_INCREF(Py_True);
		    return Py_True;
		} else {
		    Py_INCREF(Py_False);
		    return Py_False;
		}
	    }

	    if(mtype == ogf_meta<int>::type()) {
		int value;
		arg.get_value(value);
		return PyLong_FromLong(long(value));
	    }

	    if(mtype == ogf_meta<unsigned int>::type()) {
		unsigned int value;
		arg.get_value(value);
		return PyLong_FromUnsignedLong((unsigned long)(value));
	    }

	    if(mtype == ogf_meta<long>::type()) {
		long value;
		arg.get_value(value);
		return PyLong_FromLong(value);
	    }

	    if(mtype == ogf_meta<unsigned long>::type()) {
		unsigned long value;
		arg.get_value(value);
		return PyLong_FromUnsignedLong(value);
	    }

	    if(mtype == ogf_meta<index_t>::type()) {
		index_t value;
		arg.get_value(value);
		return PyLong_FromLong(long(value));
	    }

	    if(mtype == ogf_meta<signed_index_t>::type()) {
		signed_index_t value;
		arg.get_value(value);
		return PyLong_FromUnsignedLong((unsigned long)(value));
	    }

	    if(mtype == ogf_meta<float>::type()) {
		float value;
		arg.get_value(value);
		return PyFloat_FromDouble(double(value));
	    }

	    if(mtype == ogf_meta<double>::type()) {
		double value;
		arg.get_value(value);
		return PyFloat_FromDouble(value);
	    }

	    PyObject* result = graphite_mat_vec_to_python(arg);
	    if(result != nullptr) {
		return result;
	    }

	    std::string value = arg.as_string();
	    return string_to_python(value);
	}

	/**********************************************************************/

	void python_tographiteargs(
	    PyObject* args, PyObject* keywords,
	    ArgList& gom_args, MetaMethod* mmethod
	) {
	    gom_args.clear();
	    if(keywords == nullptr) {
		if(mmethod == nullptr) {
		    // Standard call (no keywords), no meta-method specified
		    // -> create an ArgList with all the arguments
		    // (used to call foreign Callable objects, e.g. LuaCallable).
		    index_t nb_args = index_t(PyTuple_Size(args));
		    for(index_t i=0; i<nb_args; i++) {
			PyObject* cur_arg = PyTuple_GetItem(args,Py_ssize_t(i));
			gom_args.create_arg(
			    "arg#" + String::to_string(i),
			    python_to_graphite(cur_arg)
			);
		    }
		} else {
		    MetaClass* mclass = mmethod->container_meta_class();
		    index_t nb_args = index_t(PyTuple_Size(args));
		    if(nb_args > mmethod->nb_args()) {
			Logger::err("GOMPy")
			    << "Graphite request: too many arguments for method "
			    << mclass->name() << "::"
			    << mmethod->name() << std::endl;
			return;
		    }
		    for(index_t i=0; i<nb_args; i++) {
			PyObject* cur_arg = PyTuple_GetItem(args,Py_ssize_t(i));
			MetaType* mtype = mmethod->ith_arg_type(i);
			gom_args.create_arg(
			    mmethod->ith_arg(i)->name(),
			    python_to_graphite(cur_arg, mtype)
			);
		    }
		    for(index_t i=nb_args; i<mmethod->nb_args(); i++) {
			if(!mmethod->ith_arg(i)->has_default_value()) {
			    Logger::err("GOMPy")
				<< "Graphite request: missing args for method "
				<< mclass->name() << "::"
				<< mmethod->name() << std::endl;
			    return;
			}
		    }
		}
	    } else {
		ogf_assert(PyDict_Check(keywords));
		Py_ssize_t nb_keywords = PyDict_Size(keywords);
		PyObject* keys = PyDict_Keys(keywords);
		Py_INCREF(keys);
		PyObject* values = PyDict_Values(keywords);
		Py_INCREF(values);
		for(int i=0; i<nb_keywords; i++) {
		    std::string argname = python_to_string(
			PyList_GetItem(keys,i)
		    );
		    MetaType* mtype = nullptr;
		    if(mmethod != nullptr) {
			MetaArg* marg = mmethod->find_arg(argname);
			if(marg != nullptr) {
			    mtype = marg->type();
			}
		    }
		    gom_args.create_arg(
			argname,
			python_to_graphite(PyList_GetItem(values,i),mtype)
		    );
		}
		Py_DECREF(keys);
		Py_DECREF(values);
	    }
	}

    /**********************************************************************/

    }
}
