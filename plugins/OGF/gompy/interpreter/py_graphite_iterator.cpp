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

#include <OGF/gompy/interpreter/py_graphite_iterator.h>
#include <OGF/gompy/interpreter/py_graphite_object.h>
#include <OGF/gompy/interpreter/interop.h>
#include <OGF/gom/types/object.h>

namespace OGF {

    namespace GOMPY {

	/**
	 * \brief A Python wrapper for iterators on Graphite objects
	 */
	struct graphite_Iterator {
	    PyObject_HEAD

	    /** \brief Pointer to object on which iteration runs. */
	    Object* object;

	    /** \brief Iteration index */
	    Numeric::int32 index;
	};

	PyTypeObject graphite_IteratorType = {
	    PyVarObject_HEAD_INIT(nullptr, 0)
	    "graphite.Iterator",        // tp_name
	    sizeof(graphite_Iterator)   // tp_basicsize
	    // The rest is initialized in init_graphite_ObjectType()
	};


	PyObject* graphite_Iterator_new(
	    PyTypeObject *type, PyObject *args, PyObject *kwds
	) {
	    geo_argused(args);
	    geo_argused(kwds);
	    graphite_Iterator *self = (graphite_Iterator *)type->tp_alloc(
		type, 0
	    );
	    self->object = nullptr;
	    self->index = -1;
	    return (PyObject*)self;
	}

	void graphite_Iterator_dealloc(PyObject* self_in) {
	    graphite_Iterator* self = (graphite_Iterator*)self_in;
	    Counted::unref(self->object);
	    self->object = nullptr;
	    self->index = -1;
	    Py_TYPE(self)->tp_free((PyObject*)self);
	}

	static PyObject* graphite_Iterator_iter(PyObject* self) {
	    return self;
	}

	static PyObject* graphite_Iterator_next(PyObject* self) {
	    graphite_Iterator* impl = (graphite_Iterator*)(self);
	    if(
		impl->object == nullptr ||
		impl->index < 0 ||
		impl->index >= int(impl->object->get_nb_elements())
	    ) {
		PyErr_SetNone(PyExc_StopIteration);
		return nullptr;
	    }
	    Any result;
	    impl->object->get_element(index_t(impl->index), result);
	    impl->index++;
	    return graphite_to_python(result);
	}

	void init_graphite_IteratorType() {
	    graphite_IteratorType.tp_dealloc     = graphite_Iterator_dealloc;
	    graphite_IteratorType.tp_flags       = Py_TPFLAGS_DEFAULT;
	    graphite_IteratorType.tp_new         = graphite_Iterator_new;
	    graphite_IteratorType.tp_iter        = graphite_Iterator_iter;
	    graphite_IteratorType.tp_iternext    = graphite_Iterator_next;
	}

	PyObject* PyGraphiteIterator_New(Object* object) {
	    PyObject* self = graphite_IteratorType.tp_alloc(
		&graphite_IteratorType,0
	    );
	    graphite_Iterator* impl = (graphite_Iterator*)(self);
	    impl->object = object;
	    impl->index = 0;
	    Counted::ref(impl->object);
	    return self;
	}
    }
}
