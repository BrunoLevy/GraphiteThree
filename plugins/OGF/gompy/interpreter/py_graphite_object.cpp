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

#include <OGF/gompy/interpreter/py_graphite_object.h>
#include <OGF/gompy/interpreter/py_graphite_metaclass.h>
#include <OGF/gompy/interpreter/py_graphite_callable.h>
#include <OGF/gompy/interpreter/py_graphite_iterator.h>
#include <OGF/gompy/interpreter/interop.h>
#include <OGF/gompy/interpreter/nl_vector_interop.h>
#include <OGF/gompy/interpreter/python_interpreter.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/types/callable.h>
#include <OGF/scene_graph/NL/vector.h>

namespace OGF {
    namespace GOMPY {


	/**
	 * \brief Tests whether a getter exists for the specified
	 *  property name.
	 * \retval true if a getter exists.
	 * \retval false otherwise.
	 */
	bool graphite_Object_has_getter(const std::string& name);

	/***************** Python wrapper for Graphite object **************/

	PyObject* graphite_Object_new(
	    PyTypeObject *type, PyObject *args, PyObject *kwds
	) {
	    geo_argused(args);
	    geo_argused(kwds);
	    graphite_Object *self = (graphite_Object *)type->tp_alloc(type, 0);
	    self->object = nullptr;
	    self->managed = true;
	    self->array_struct = nullptr;
	    return (PyObject*)self;
	}

	void graphite_Object_dealloc(PyObject* self_in) {
	    geo_debug_assert(PyGraphite_Check(self_in));
	    graphite_Object* self = (graphite_Object*)self_in;
	    if(self->managed) {
		Counted::unref(self->object);
	    }
	    self->object = nullptr;
	    Py_TYPE(self)->tp_free((PyObject*)self);
	    Py_XDECREF(self->array_struct);
	    self->array_struct = nullptr;
	}

	static PyObject* graphite_Object_richcompare(
	    PyObject* self, PyObject* rhs, int op
	) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    Object* other = nullptr;

	    if(PyGraphite_Check(rhs)) {
		other = PyGraphite_GetObject(rhs);
	    } else {
		if(rhs != Py_None) {
		    Py_RETURN_NOTIMPLEMENTED;
		}
	    }

	    bool result = true;
	    if(object == nullptr || other == nullptr) {
		if(op == Py_EQ) {
		    result = (object == other);
		} else if (op == Py_NE) {
		    result = (object != other);
		} else {
		    Py_RETURN_NOTIMPLEMENTED;
		}
	    } else {
		Sign s = object->compare(other);
		switch(op) {
		case Py_LT: result = (int(s) <  0); break ;
		case Py_LE: result = (int(s) <= 0); break ;
		case Py_EQ: result = (int(s) == 0); break ;
		case Py_NE: result = (int(s) != 0); break ;
		case Py_GE: result = (int(s) >= 0); break ;
		case Py_GT: result = (int(s) >  0); break ;
		default: geo_assert_not_reached;
		}
	    }
	    PyObject* pyresult = result ? Py_True : Py_False;
	    Py_INCREF(pyresult);
	    return pyresult;
	}

	static Py_hash_t graphite_Object_hash(PyObject* self) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    return Py_hash_t(object);
	}

	static PyObject* graphite_Object_getattro(
	    PyObject* self, PyObject* name_in
	) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    if(object == nullptr) {
		Logger::err("GOMPy")
		    << "tried to get attribute from nullptr object"
		    << std::endl;
		Py_INCREF(Py_None);
		return Py_None;
	    }
	    std::string name = python_to_string(name_in);

	    // Case 1: regular property
	    MetaProperty* mprop = object->meta_class()->find_property(name);
	    if(mprop != nullptr) {
		Any value;
		if(!object->get_property(name, value)) {
		    Py_INCREF(Py_None);
		    return Py_None;
		}
		return graphite_to_python(value, mprop->type());
	    }


	    // Case 2: assembling a graphite request (object.method to be
	    // called right after).
	    MetaMethod* mmethod = object->meta_class()->find_method(name);
	    if(
		mmethod != nullptr &&
		dynamic_cast<MetaConstructor*>(mmethod) == nullptr
	    ) {
		// If object is an interpreter, do not do reference counting,
		// else this creates circular references, preventing objects from
		// being deallocated.
		bool managed = (dynamic_cast<Interpreter*>(object) == nullptr);
		PyObject* result = PyGraphiteObject_New(
		    new Request(object, mmethod, managed)
		);
		Py_INCREF(result);
		return result;
	    }

	    // Case 3: resolving symbol in Scope.
	    Scope* scope = dynamic_cast<Scope*>(object);
	    if(scope != nullptr && !graphite_Object_has_getter(name)) {
		Any prop = scope->resolve(name);
		PyObject* result = graphite_to_python(prop);
		Py_INCREF(result);
		return result;
	    }

	    // Case 4: object is a meta class, attribute may be a meta member
	    MetaClass* object_as_meta_class = dynamic_cast<MetaClass*>(object);
	    if(object_as_meta_class != nullptr) {
		MetaMember* mmember = object_as_meta_class->find_member(name);
		if(mmember != nullptr) {
		    PyObject* result = PyGraphiteObject_New(mmember);
		    Py_INCREF(result);
		    return result;
		}
	    }

	    // All other cases: use Python generic attribute mechanism.
	    PyObject* result = PyObject_GenericGetAttr(self, name_in);
	    return(result);
	}


	static int graphite_Object_setattro(
	    PyObject* self, PyObject* name_in, PyObject* value
	) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    if(object == nullptr) {
		return -1;
	    }
	    std::string name = python_to_string(name_in);
	    MetaType* mtype = nullptr;
	    MetaProperty* mprop = object->meta_class()->find_property(name);
	    if(mprop != nullptr) {
		mtype = mprop->type();
	    }
	    if(!object->set_property(name, python_to_graphite(value, mtype))) {
		return -1;
	    }
	    return 0;
	}

	static PyObject* graphite_str(PyObject* self) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    if(object == nullptr) {
		return string_to_python("<null GOM Object>");
	    }

	    MetaClass* mclass = object->meta_class();

	    std::ostringstream out;
	    out << "[GOM " << mclass->name() << "]";
	    return string_to_python(out.str());
	}

	PyObject* graphite_call(
	    PyObject* self, PyObject* args, PyObject* keywords
	) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);

	    if(object == nullptr) {
		Logger::err("GOMPy")
		    << "Graphite request: missing object" << std::endl;
		Py_INCREF(Py_None);
		return Py_None;
	    }


	    Callable_var c;
	    MetaClass*  mclass = dynamic_cast<MetaClass*>(object);
	    MetaMethod* method = nullptr;

	    // If target is a meta_class, try to invoke constructor
	    if(mclass != nullptr) {
		method = mclass->meta_class()->find_method("create");
		if(method != nullptr) {
		    c = new Request(mclass, method);
		}
	    } else {
		// Else, test if target is a callable
		c = dynamic_cast<Callable*>(object);
		Request* r = dynamic_cast<Request*>(c.get());
		if(r != nullptr) {
		    method = r->method();
		}
	    }

	    if(c.is_null()) {
		Logger::err("GOMPy")
		    << "Error in graphite_call(): target is not a Callable"
		    << std::endl;
		Py_INCREF(Py_None);
		return Py_None;
	    }


	    ArgList gom_args;

	    // Special case: method has a single argument of type ArgList
	    // -> pack all the arguments in a ArgList
	    if(
		method != nullptr &&
		method->nb_args() == 1 &&
		method->ith_arg_type(0) == ogf_meta<OGF::ArgList>::type()
	    ) {
		python_tographiteargs(args, keywords, gom_args);
	    } else {
		// Regular case: identify each individual argument according
		// to method declaration. Add default values if need be.
		python_tographiteargs(args, keywords, gom_args, method);
	    }

	    Any result;
	    bool ok = c->invoke(gom_args, result);
	    if(!ok) {
		if(method != nullptr) {
		    Logger::err("GOMPy")
			<< "error while invoking " +
			method->container_meta_class()->name() +
			"::" + method->name()
			<< std::endl;
		}
		Py_INCREF(Py_None);
		return Py_None;
	    }
	    return graphite_to_python(
		result, method == nullptr ? nullptr : method->return_type()
	    );
	}

	static Py_ssize_t graphite_array_len(PyObject* self) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    if(object == nullptr) {
		Logger::err("GOMPy") << "tried to index null object"
				     << std::endl;
		return 0;
	    }
	    return Py_ssize_t(object->get_nb_elements());
	}

	static PyObject* graphite_array_index(PyObject* self, PyObject* index) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    Any result;
	    if(!PyLong_Check(index)) {
		Logger::err("GOMPy") << "index is not an integer"
				     << std::endl;
		return graphite_to_python(result);
	    }
	    if(object == nullptr) {
		Logger::err("GOMPy") << "tried to index null object"
				     << std::endl;
		return graphite_to_python(result);
	    }
	    object->get_element(
		index_t(PyLong_AsLong(index)), result
	    );
	    return graphite_to_python(result);
	}

	static int graphite_array_ass_index(
	    PyObject* self, PyObject* index, PyObject* value_in
	) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    if(!PyLong_Check(index)) {
		Logger::err("GOMPy") << "index is not an integer"
				     << std::endl;
		return -1;
	    }
	    if(object == nullptr) {
		Logger::err("GOMPy") << "tried to index null object"
				     << std::endl;
		return -1;
	    }
	    Any value = python_to_graphite(value_in);
	    object->set_element(
		index_t(PyLong_AsLong(index)), value
	    );
	    return 0;
	}

	static PyObject* graphite_dir_method(PyObject* self, PyObject* args_in) {
	    geo_argused(args_in);
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    PyObject* result = nullptr;

	    if(object == nullptr) {
		Logger::err("GOMpy") << "dir() called on null object"
				     << std::endl;
		return result;
	    }

	    Scope* scope = dynamic_cast<Scope*>(object);
	    if(scope != nullptr) {
		std::vector<std::string> names;
		scope->list_names(names);
		result = PyTuple_New(Py_ssize_t(names.size()));
		FOR(i,names.size()) {
		    PyTuple_SetItem(
			result,
			Py_ssize_t(i),
			string_to_python(names[i])
		    );
		}
		Py_INCREF(result);
		return result;
	    }

	    bool query_superclasses = false;
	    MetaClass* meta_class = dynamic_cast<MetaClass*>(object);
	    if(meta_class == nullptr) {
		query_superclasses = true;
		meta_class = object->meta_class();
	    }

	    if(meta_class == nullptr) {
		Py_INCREF(Py_None);
		return Py_None;
	    }

	    // Return all members that are not metaconstructors.
	    index_t nb = 0;
	    FOR(i, meta_class->nb_members(query_superclasses)) {
		MetaMember* mm = meta_class->ith_member(i,query_superclasses);
		if(dynamic_cast<MetaConstructor*>(mm) == nullptr) {
		    ++nb;
		}
	    }
	    result = PyTuple_New(Py_ssize_t(nb));
	    index_t cur=0;
	    FOR(i, meta_class->nb_members(query_superclasses)) {
		MetaMember* mm = meta_class->ith_member(i,query_superclasses);
		if(dynamic_cast<MetaConstructor*>(mm) == nullptr) {
		    PyTuple_SetItem(
			result,
			cur,
			string_to_python(mm->name())
		    );
		    ++cur;
		}
	    }

	    Py_INCREF(result);
	    return result;
	}

	static PyObject* graphite_Object_iter(PyObject* self) {
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    return PyGraphiteIterator_New(object);
	}

	/**
	 * \brief Methods definition for Python wrapper around Graphite object.
	 */
	static PyMethodDef graphite_Object_methods[] = {
	    {
		"__dir__",
		graphite_dir_method,
		METH_NOARGS,
		"Implementation of dir() for Graphite objects"
	    },
	    {
		nullptr, /* ml_name */
		nullptr, /* ml_meth */
		0,       /* ml_flags */
		nullptr  /* ml_doc */
	    }
	};


	static PyObject* graphite_get_class(PyObject* self, void* closure) {
	    geo_argused(closure);
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    PyObject* result = PyGraphiteObject_New(object->meta_class());
	    Py_INCREF(result);
	    return result;
	}

	static PyObject* graphite_get_bases(PyObject* self, void* closure) {
	    geo_argused(closure);
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    MetaClass* mclass = dynamic_cast<MetaClass*>(object);
	    if(mclass == nullptr) {
		Logger::err("GOMPy") << "__bases__ queried on non-class object"
				     << std::endl;
		Py_INCREF(Py_None);
		return Py_None;
	    }
	    MetaClass* superclass = mclass->super_class();
	    PyObject* result = PyTuple_New(superclass != nullptr);
	    if(superclass != nullptr) {
		PyTuple_SetItem(result, 0, PyGraphiteObject_New(superclass));
	    }
	    Py_INCREF(result);
	    return result;
	}

	static PyObject* graphite_get_interfaces(PyObject* self, void* closure) {
	    geo_argused(closure);
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    PyObject* result = PyGraphiteObject_New(
		new InterfaceScope(object)
	    );
	    Py_INCREF(result);
	    return result;
	}

	static PyObject* graphite_get_array_struct(
	    PyObject* self_in, void* closure
	) {
	    geo_argused(closure);
	    geo_debug_assert(PyGraphite_Check(self_in));
	    graphite_Object* self = reinterpret_cast<graphite_Object*>(self_in);
	    Py_XINCREF(self->array_struct);
	    return self->array_struct;
	}

	PyObject* graphite_get_doc(PyObject* self, void* closure) {
	    geo_argused(closure);
	    geo_debug_assert(PyGraphite_Check(self));
	    Object* object = PyGraphite_GetObject(self);
	    std::string result_string;
	    if(object == nullptr) {
		result_string = "null";
	    } else {
		result_string = object->get_doc();
	    }
	    PyObject* result = string_to_python(result_string);
	    Py_INCREF(result);
	    return result;
	}

	PyGetSetDef graphite_Object_getsets[] = {
	    {
		const_cast<char*>("__class__"),
		graphite_get_class,
		nullptr,
		nullptr,
		nullptr
	    },
	    {
		const_cast<char*>("__bases__"),
		graphite_get_bases,
		nullptr,
		nullptr,
		nullptr
	    },
	    {
		const_cast<char*>("__array_struct__"),
		graphite_get_array_struct,
		nullptr,
		nullptr,
		nullptr
	    },
	    {
		const_cast<char*>("I"),
		graphite_get_interfaces,
		nullptr,
		nullptr,
		nullptr
	    },
	    {
		const_cast<char*>("__doc__"),
		graphite_get_doc,
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

	bool graphite_Object_has_getter(const std::string& name) {
	    for(index_t i=0; ;++i) {
		if(graphite_Object_getsets[i].name == nullptr) {
		    break;
		}
		if(graphite_Object_getsets[i].name == name) {
		    return true;
		}
	    }
	    return false;
	}

	/**
	 * \brief Methods definition for array access in
	 *  Python wrapper around Graphite object.
	 */
	static PyMappingMethods graphite_MappingMethods = {
	    graphite_array_len,      /* mp_length */
	    graphite_array_index,    /* mp_subscript */
	    graphite_array_ass_index /* mp_ass_subscript */
	};

	PyTypeObject graphite_ObjectType = {
	    PyVarObject_HEAD_INIT(nullptr, 0)
	    "graphite.Object",        // tp_name
	    sizeof(graphite_Object)   // tp_basicsize
	    // The rest is initialized in init_graphite_ObjectType()
	};

	void init_graphite_ObjectType() {
	    graphite_ObjectType.tp_dealloc     = graphite_Object_dealloc;
	    graphite_ObjectType.tp_as_mapping  = &graphite_MappingMethods;
	    graphite_ObjectType.tp_str         = graphite_str;
	    graphite_ObjectType.tp_getattro    = graphite_Object_getattro;
	    graphite_ObjectType.tp_setattro    = graphite_Object_setattro;
	    graphite_ObjectType.tp_flags       = Py_TPFLAGS_DEFAULT;
	    graphite_ObjectType.tp_methods     = graphite_Object_methods;
	    graphite_ObjectType.tp_getset      = graphite_Object_getsets;
	    graphite_ObjectType.tp_new         = graphite_Object_new;
	    graphite_ObjectType.tp_richcompare = graphite_Object_richcompare;
	    graphite_ObjectType.tp_hash        = graphite_Object_hash;
	    graphite_ObjectType.tp_iter        = graphite_Object_iter;
	}

	/***************************************************************/

	PyObject* PyGraphiteObject_New(Object* object, bool managed) {
	    PyObject *self = nullptr;
	    PyTypeObject* type = nullptr;
	    bool is_meta_class = false;

	    if(dynamic_cast<MetaClass*>(object) != nullptr) {
		type = &graphite_MetaClassType;
		is_meta_class = true;
	    } else if(dynamic_cast<Callable*>(object) != nullptr) {
		type = &graphite_CallableType;
	    } else {
		type = &graphite_ObjectType;
	    }
	    self = type->tp_alloc(type, 0);

	    if(is_meta_class) {
		graphite_MetaClass* impl = reinterpret_cast<graphite_MetaClass*>(
		    self
		);
		impl->object = object;

		// TODO: understand why PyType_ready() does not initialize
		// mro (method resolution order). For now I set it as an
		// empty tuple (and this avoids crashing)
		impl->head.tp_mro = PyTuple_New(0);
		Py_INCREF(impl->head.tp_mro);

		impl->magic = graphite_MetaClass_MAGIC;
	    } else {
		graphite_Object* impl = reinterpret_cast<graphite_Object*>(
		    self
		);
		impl->object = object;
		impl->managed = managed;
		if(impl->managed) {
		    Counted::ref(impl->object);
		}

		// If object is a vector, create information for interop
		// with numpy.
		NL::Vector* object_as_vector = dynamic_cast<NL::Vector*>(object);
		if(object_as_vector != nullptr) {
		    PyObject* array_interface =
			create_array_interface(object_as_vector);
		    Py_INCREF(array_interface);
		    impl->array_struct = array_interface;
		}

		impl->magic = graphite_Object_MAGIC;
	    }

	    return self;
	}

	bool PyGraphite_Check(PyObject* obj) {
	    return PyObject_IsInstance(
		obj, (PyObject*)&graphite_ObjectType
	    ) != 0;
	}

	bool PyGraphiteObject_Check(PyObject* obj) {
	    return PyGraphite_Check(obj) && !PyGraphiteMetaClass_Check(obj);
	}

	Object* PyGraphite_GetObject(PyObject* obj) {
	    geo_debug_assert(PyGraphite_Check(obj));
	    Object* result = nullptr;
	    if(PyGraphiteMetaClass_Check(obj)) {
		graphite_MetaClass* impl =
		    reinterpret_cast<graphite_MetaClass*>(obj);
		geo_assert(impl->magic == graphite_MetaClass_MAGIC);
		result = impl->object;
	    } else {
		graphite_Object* impl =
		    reinterpret_cast<graphite_Object*>(obj);
		geo_assert(impl->magic == graphite_Object_MAGIC);
		result = impl->object;
	    }
	    return result;
	}

	/********************************************************************/
    }
}
