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

#include <OGF/gompy/interpreter/py_graphite_metaclass.h>
#include <OGF/gompy/interpreter/py_graphite_object.h>
#include <OGF/gompy/interpreter/interop.h>
#include <OGF/gom/reflection/meta_class.h>

namespace OGF {
    namespace GOMPY {

	static PyObject* graphite_MetaClass_new(
	    PyTypeObject *type, PyObject *args, PyObject *kwds
	) {
	    geo_argused(args);
	    geo_argused(kwds);
	    graphite_MetaClass *self =
		reinterpret_cast<graphite_MetaClass *>(type->tp_alloc(type, 0));
	    self->object = nullptr;
	    // self->weakrefs = nullptr; // TODO: needed ?
	    return reinterpret_cast<PyObject*>(self);
	}

	static void graphite_MetaClass_dealloc(PyObject* self_in) {
	    geo_debug_assert(PyGraphiteMetaClass_Check(self_in));
	    graphite_MetaClass* self =
		reinterpret_cast<graphite_MetaClass*>(self_in);
	    Counted::unref(self->object);
	    self->object = nullptr;
	    Py_TYPE(self)->tp_free(self_in);
	}

	PyObject* graphite_MetaClass_get_name(PyObject* self, void* closure) {
	    geo_argused(closure);
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    MetaClass* mclass = dynamic_cast<MetaClass*>(object);
	    std::string result_string;
	    if(mclass == nullptr) {
		result_string = "null";
	    } else {
		result_string = mclass->name();
	    }
	    PyObject* result = string_to_python(result_string);
	    Py_INCREF(result);
	    return result;
	}


	PyGetSetDef graphite_MetaClass_getsets[] = {
	    {
		const_cast<char*>("__name__"),
		graphite_MetaClass_get_name,
		nullptr,
		nullptr,
		nullptr
	    },
	    {
		nullptr, /* name */
		nullptr, /* getter */
		nullptr, /* setter */
		nullptr, /* doc */
		nullptr  /* closure */
	    }
	};


	PyTypeObject graphite_MetaClassType = {
	    PyVarObject_HEAD_INIT(nullptr, 0)
	    "graphite.MetaClass",      // tp_name
	    sizeof(graphite_MetaClass) // tp_basicsize
	    // The rest is initialized by init_graphite_MetaClassType()
	};

	void init_graphite_MetaClassType() {
	    graphite_MetaClassType.tp_call       = graphite_call;
	    graphite_MetaClassType.tp_dealloc    = graphite_MetaClass_dealloc;

	    // TYPE_SUBCLASS so that Python knows it is a type, and
	    // HEAPTYPE, so that Python knows it is not a static type
	    //   \__> this one causes problems, graphite_MetaClassType then
	    // needs to be a PyHeapTypeObject, to be investigated.
	    graphite_MetaClassType.tp_flags      = Py_TPFLAGS_DEFAULT |
		                                   Py_TPFLAGS_TYPE_SUBCLASS
                                                /* | Py_TPFLAGS_HEAPTYPE */ ;

	    graphite_MetaClassType.tp_getset     = graphite_MetaClass_getsets;
	    graphite_MetaClassType.tp_base       = &graphite_ObjectType;
	    graphite_MetaClassType.tp_new        = graphite_MetaClass_new;

	    // Note: not using Py_TPFLAGS_MANAGED_WEAKREF, but we
	    // need tp_weaklistoffset to be set else it crashes
	    // disclaimer: I do not understand what weakrefs are !!
	    graphite_MetaClassType.tp_weaklistoffset = offsetof(
		graphite_MetaClass, weakrefs
	    );

	}

	bool PyGraphiteMetaClass_Check(PyObject* obj) {
	    return obj->ob_type == &graphite_MetaClassType;
	    /*
	    return PyObject_IsInstance(
		obj, (PyObject*)&graphite_MetaClassType
	    ) != 0; // -> infinite recursion (TODO: understand why)
	    */
	}

    }
}
