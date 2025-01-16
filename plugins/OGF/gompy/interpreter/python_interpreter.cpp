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


#include <OGF/gompy/interpreter/python_interpreter.h>
#include <OGF/gompy/interpreter/python.h>
#include <OGF/gompy/interpreter/py_graphite_object.h>
#include <OGF/gompy/interpreter/py_graphite_callable.h>
#include <OGF/gompy/interpreter/interop.h>
#include <OGF/gompy/interpreter/nl_vector_interop.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/basic/os/file_manager.h>
#include <OGF/basic/modules/modmgr.h>
#include <OGF/scene_graph/NL/vector.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/command_line.h>

/*************************************************************************/

// TODO:
//  - isubclass(), isinstance() for NL Vectors (jax complains)

/*************************************************************************/

namespace {
    using namespace OGF;
    using namespace OGF::GOMPY;

    /*************************************************************************/

    /**
     * \brief A Python wrapper for Graphite objects.
     */
    struct graphite_Object {
        PyObject_HEAD

	/** \brief Pointer to the implementation. */
        Object* object;

        /** \brief true if reference-counted, false otherwise. */
	bool managed;

	/** \brief Pointer to array interface or nullptr. */
	PyObject* array_struct;
    };


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

    PyObject* graphite_Object_richcompare(
        PyObject* self_in, PyObject* rhs_in, int op
    ) {
        geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Object* object = self->object;
        Object* other = nullptr;

        if(PyGraphite_Check(self_in)) {
            graphite_Object* rhs = (graphite_Object*)rhs_in;
            other = rhs->object;
        } else {
            if(self_in != Py_None) {
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

    Py_hash_t graphite_Object_hash(PyObject* self_in) {
        geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Object* object = self->object;
        return Py_hash_t(object);
    }

    PyObject* graphite_Object_getattro(PyObject* self_in, PyObject* name_in) {
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Object* object = self->object;
        if(object == nullptr) {
            Logger::err("GOMPy") << "tried to get attribute from nullptr object"
				 << std::endl;
	    Py_INCREF(Py_None);
            return Py_None;
        }
        std::string name = python_to_string(name_in);

	// Case 1: regular property
        MetaProperty* mprop = self->object->meta_class()->find_property(name);
        if(mprop != nullptr) {
            Any value;
            if(!self->object->get_property(name, value)) {
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
	    // If object is an interpreter, do not do reference counting, else
	    // this creates circular references, preventing objects from
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
	PyObject* result = PyObject_GenericGetAttr(self_in, name_in);
	return(result);
    }


    int graphite_Object_setattro(
        PyObject* self_in, PyObject* name_in, PyObject* value
    ) {
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
        if(self->object == nullptr) {
            return -1;
        }
        std::string name = python_to_string(name_in);
	MetaType* mtype = nullptr;
	MetaProperty* mprop = self->object->meta_class()->find_property(name);
	if(mprop != nullptr) {
	    mtype = mprop->type();
	}
	if(!self->object->set_property(name, python_to_graphite(value, mtype))) {
            return -1;
        }
        return 0;
    }

    PyObject* graphite_str(PyObject* self_in) {
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;

        if(self->object == nullptr) {
            return string_to_python("<null GOM Object>");
        }

        MetaClass* mclass = self->object->meta_class();

        std::ostringstream out;
        out << "[GOM " << mclass->name() << "]";
        return string_to_python(out.str());
    }

    PyObject* graphite_call(
        PyObject* self_in, PyObject* args, PyObject* keywords
    ) {
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;

        if(self->object == nullptr) {
            Logger::err("GOMPy")
                << "Graphite request: missing object" << std::endl;
	    Py_INCREF(Py_None);
            return Py_None;
        }


	Callable_var c;
	MetaClass*  mclass = dynamic_cast<MetaClass*>(self->object);
	MetaMethod* method = nullptr;

	// If target is a meta_class, try to invoke constructor
	if(mclass != nullptr) {
	    method = mclass->meta_class()->find_method("create");
	    if(method != nullptr) {
		c = new Request(mclass, method);
	    }
	} else {
	    // Else, test if target is a callable
	    c = dynamic_cast<Callable*>(self->object);
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

    Py_ssize_t graphite_array_len(PyObject* self_in) {
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	if(self->object == nullptr) {
	    Logger::err("GOMPy") << "tried to index null object"
				 << std::endl;
	    return 0;
	}
	return Py_ssize_t(self->object->get_nb_elements());
    }

    PyObject* graphite_array_index(PyObject* self_in, PyObject* index) {
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Any result;
	if(!PyLong_Check(index)) {
	    Logger::err("GOMPy") << "index is not an integer"
				 << std::endl;
	    return graphite_to_python(result);
	}
	if(self->object == nullptr) {
	    Logger::err("GOMPy") << "tried to index null object"
				 << std::endl;
	    return graphite_to_python(result);
	}
	self->object->get_element(
	    index_t(PyLong_AsLong(index)), result
	);
	return graphite_to_python(result);
    }

    int graphite_array_ass_index(
	PyObject* self_in, PyObject* index, PyObject* value_in
    ) {
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	if(!PyLong_Check(index)) {
	    Logger::err("GOMPy") << "index is not an integer"
				 << std::endl;
	    return -1;
	}
	if(self->object == nullptr) {
	    Logger::err("GOMPy") << "tried to index null object"
				 << std::endl;
	    return -1;
	}
	Any value = python_to_graphite(value_in);
	self->object->set_element(
	    index_t(PyLong_AsLong(index)), value
	);
	return 0;
    }

    PyObject* graphite_dir_method(PyObject* self_in, PyObject* args_in) {
	geo_argused(args_in);
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	PyObject* result = nullptr;

	if(self->object == nullptr) {
	    Logger::err("GOMpy") << "dir() called on null object"
				 << std::endl;
	    return result;
	}

	Scope* scope = dynamic_cast<Scope*>(self->object);
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
	MetaClass* meta_class = dynamic_cast<MetaClass*>(self->object);
	if(meta_class == nullptr) {
	    query_superclasses = true;
	    meta_class = self->object->meta_class();
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

    /**
     * \brief Methods definition for Python wrapper around Graphite object.
     */
    PyMethodDef graphite_Object_methods[] = {
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


    PyObject* graphite_get_class(PyObject* self_in, void* closure) {
	geo_argused(closure);
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Object* object = self->object;
	PyObject* result = PyGraphiteObject_New(object->meta_class());
	Py_INCREF(result);
	return result;
    }

    PyObject* graphite_get_bases(PyObject* self_in, void* closure) {
	geo_argused(closure);
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Object* object = self->object;
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

    PyObject* graphite_get_interfaces(PyObject* self_in, void* closure) {
	geo_argused(closure);
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Object* object = self->object;
	PyObject* result = PyGraphiteObject_New(
	    new InterfaceScope(object)
	);
	Py_INCREF(result);
	return result;
    }

    PyObject* graphite_get_array_struct(PyObject* self_in, void* closure) {
	geo_argused(closure);
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Py_XINCREF(self->array_struct);
	return self->array_struct;
    }

    PyObject* graphite_get_doc(PyObject* self_in, void* closure) {
	geo_argused(closure);
	geo_debug_assert(PyGraphite_Check(self_in));
	graphite_Object* self = (graphite_Object*)self_in;
	Object* object = self->object;
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
    PyMappingMethods graphite_MappingMethods = {
	graphite_array_len,      /* mp_length */
	graphite_array_index,    /* mp_subscript */
	graphite_array_ass_index /* mp_ass_subscript */
    };


    /**
     * \brief Clears all the field of a Python type object except
     *  the header.
     * \details I prefer to do that rather than explictly initializing
     *  all the fields in the struct declaration, because the fields
     *  are different in different Python versions. Here we got a portable
     *  way of clearing all the fields (then setting only the fields that
     *  we use).
     * \param[in] obj a pointer to the Python type object to be
     *  cleared.
     */
    void clear_PyTypeObject(PyTypeObject* obj) {
	void* start = &(obj->tp_itemsize);
	size_t len = sizeof(PyTypeObject) -
	    size_t(Memory::pointer(start)-Memory::pointer(obj));
	Memory::clear(start,len);
    }


    /**
     * \brief Class definition for Python wrapper
     *  around Graphite object.
     */
    PyTypeObject graphite_ObjectType = {
        PyVarObject_HEAD_INIT(nullptr, 0)
        "graphite.Object",        // tp_name
        sizeof(graphite_Object)   // tp_basicsize
	// The rest is left uninitialized, and is set to zero using
	// clear_PyTypeObject().
    };

    /**
     * \brief Function to initialize graphite_ObjectType
     * \details I prefer to do that by clearing the structure
     *  then initializing each field explicitly, because Python
     *  keeps changing the definition of PyTypeObject. Initializing
     *  all the fields of PyTypeObject would require lots of #ifdef
     *  statements for testing Python version.
     */
    void init_graphite_ObjectType() {
	clear_PyTypeObject(&graphite_ObjectType);
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
    }

    /**************************************************/

    PyGetSetDef graphite_Callable_getsets[] = {
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

    /**
     * \brief Class definition for Python wrapper
     *  around Graphite object.
     */
    PyTypeObject graphite_CallableType = {
        PyVarObject_HEAD_INIT(nullptr, 0)
        "graphite.Callable",      // tp_name
        sizeof(graphite_Object)   // tp_basicsize
	// The rest is left uninitialized, and is set to zero using
	// clear_PyTypeObject().
    };

    /**
     * \brief Function to initialize graphite_CallableType
     * \details I prefer to do that by clearing the structure
     *  then initializing each field explicitly, because Python
     *  keeps changing the definition of PyTypeObject. Initializing
     *  all the fields of PyTypeObject would require lots of #ifdef
     *  statements for testing Python version.
     */
    void init_graphite_CallableType() {
	/*
	 * Declaring graphite_call() in graphite_ObjectType
	 * would have done the job, but it is cleaner like that. In
	 * addition, having non-nullptr tp_call in graphite objects
	 * made the autocompleter systematically add '(' to object
	 * names.
	 */
	graphite_CallableType.tp_dealloc    = graphite_Object_dealloc;
	graphite_CallableType.tp_call       = graphite_call;
	graphite_CallableType.tp_flags      = Py_TPFLAGS_DEFAULT;
	graphite_CallableType.tp_getset     = graphite_Callable_getsets;
	graphite_CallableType.tp_base       = &graphite_ObjectType;
	graphite_CallableType.tp_new        = graphite_Object_new;
    }

    /**************************************************/

    /**
     * \brief Class definition for Python wrapper
     *  around Graphite object.
     */
    PyTypeObject graphite_MetaClassType = {
        PyVarObject_HEAD_INIT(nullptr, 0)
        "graphite.MetaClass",     // tp_name
        sizeof(graphite_Object)   // tp_basicsize
	// The rest is left uninitialized, and is set to zero using
	// clear_PyTypeObject().
    };

    /**
     * \brief Function to initialize graphite_MetaClassType
     * \details I prefer to do that by clearing the structure
     *  then initializing each field explicitly, because Python
     *  keeps changing the definition of PyTypeObject. Initializing
     *  all the fields of PyTypeObject would require lots of #ifdef
     *  statements for testing Python version.
     */
    void init_graphite_MetaClassType() {
	/*
	 * Declaring graphite_call() in graphite_ObjectType
	 * would have done the job, but it is cleaner like that. In
	 * addition, having non-nullptr tp_call in graphite objects
	 * made the autocompleter systematically add '(' to object
	 * names.
	 */
	graphite_MetaClassType.tp_call       = graphite_call;
	graphite_MetaClassType.tp_dealloc    = graphite_Object_dealloc;
	graphite_MetaClassType.tp_flags      = Py_TPFLAGS_DEFAULT;
	graphite_MetaClassType.tp_getset     = graphite_Object_getsets;
	graphite_MetaClassType.tp_base       = &graphite_ObjectType;
	graphite_MetaClassType.tp_new        = graphite_Object_new;
    }


    /*****************************************************************/

    PyObject* graphite_interpreter(PyObject* self_in, PyObject* args_in) {
	geo_argused(self_in);
	geo_argused(args_in);
	Interpreter* interpreter = Interpreter::instance_by_language("Python");
	PyObject* result = PyGraphiteObject_New(interpreter);
	Py_INCREF(result);
	return result;
    }

    PyMethodDef graphite_module_methods[] = {
	{
	    "interpreter",
	    graphite_interpreter,
	    METH_NOARGS,
	    "gets the interpreter"
	},
        {
            nullptr, /* ml_name */
            nullptr, /* ml_meth */
            0,       /* ml_flags */
            nullptr  /* ml_doc */
        }
    };

    static struct PyModuleDef graphite_moduledef = {
        PyModuleDef_HEAD_INIT,
        "GOM",                   /* m_name */
        "Graphite Object Model", /* m_doc */
        -1,                      /* m_size */
        graphite_module_methods, /* m_methods */
        nullptr,                 /* m_reload */
        nullptr,                 /* m_traverse */
        nullptr,                 /* m_clear */
        nullptr                  /* m_free */
    };

    PyMODINIT_FUNC PyInit_gom(void);
    PyMODINIT_FUNC PyInit_gom() {
	init_graphite_ObjectType();
	init_graphite_CallableType();
	init_graphite_MetaClassType();
        PyObject* m = PyModule_Create(&graphite_moduledef);
        if(m == nullptr) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        if (PyType_Ready(&graphite_ObjectType) < 0) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        if (PyType_Ready(&graphite_CallableType) < 0) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        if (PyType_Ready(&graphite_MetaClassType) < 0) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        Py_INCREF(&graphite_ObjectType);
        PyModule_AddObject(m, "Object",  (PyObject *)&graphite_ObjectType);
        return m;
    }

    /*****************************************************************/

}

namespace OGF {

    namespace GOMPY {

    /**************************** Utilities ****************************/

	PyObject* PyGraphiteObject_New(Object* object, bool managed) {
	    graphite_Object *self = nullptr;
	    PyTypeObject* type = nullptr;
	    if(dynamic_cast<MetaClass*>(object) != nullptr) {
		type = &graphite_MetaClassType;
	    } else if(dynamic_cast<Callable*>(object) != nullptr) {
		type = &graphite_CallableType;
	    } else {
		type = &graphite_ObjectType;
	    }
	    self = (graphite_Object *)type->tp_alloc(type, 0);
	    self->object = object;
	    self->managed = managed;
	    if(self->managed) {
		Counted::ref(self->object);
	    }

	    // If object is a vector, create information for interop
	    // with numpy.
	    NL::Vector* object_as_vector =  dynamic_cast<NL::Vector*>(object);
	    if(object_as_vector != nullptr) {
		PyObject* array_interface =
		    create_array_interface(object_as_vector);
		Py_INCREF(array_interface);
		self->array_struct = array_interface;
	    }

	    return reinterpret_cast<PyObject*>(self);
	}

	bool PyGraphite_Check(PyObject* obj) {
	    return PyObject_IsInstance(
		obj, (PyObject*)&graphite_ObjectType
	    ) != 0;
	}

	Object* PyGraphite_GetObject(PyObject* obj) {
	    geo_debug_assert(PyGraphite_Check(obj));
	    return reinterpret_cast<graphite_Object*>(obj)->object;
	}
    }

    /*****************************************************************/

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

    /*****************************************************************/

    PythonInterpreter::PythonInterpreter() : main_module_(nullptr) {
	use_embedded_interpreter_ = (Py_IsInitialized() == 0);

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);

	if(use_embedded_interpreter_) {
	    PyConfig config;
	    PyConfig_InitPythonConfig(&config);


	    // If there are Python subdirectories in OGF_PATH, add them
	    // to Python path.
	    const std::vector<std::string>& ogf_path =
		FileManager::instance()->ogf_path();
	    if(
		ogf_path.size() > 0 &&
		FileSystem::is_directory(ogf_path[0] + "/lib/Python")
	    ) {
		Logger::out("GOMpy")
		    << "Found local python lib. directory in Graphite: "
		    << ogf_path[0] + "/lib/Python"
		    << std::endl;
		Logger::out("GOMpy")
		    << " -> Setting python path there." << std::endl;
		std::string python_path;
		for(index_t i=0; i<ogf_path.size(); ++i) {
		    if(python_path.length() != 0) {
#ifdef GEO_OS_WINDOWS
			python_path += ';';
#else
			python_path += ':';
#endif
		    }
		    python_path += (ogf_path[i] + "/lib/Python");
		}
		config.pythonpath_env =
		    Py_DecodeLocale(python_path.c_str(), nullptr);
	    }


	    // Needed since our module is not in a separate shared object.
	    PyImport_AppendInittab("GOM", PyInit_gom);
	    Py_InitializeFromConfig(&config);

	    // PyMem_RawFree(config.pythonpath_env);
               // TODO: do we need to do that ?

	    // What follows is the low-level equivalent to:
	    //PyRun_SimpleString("import GOM");

	    PyObject* gom_module = PyImport_ImportModule("GOM");
	    main_module_ = PyImport_AddModule("__main__");
	    PyObject_SetAttrString(main_module_, "GOM", gom_module);
	    Py_XDECREF(gom_module);

	    PyObject* gom = PyGraphiteObject_New(this, false);
	    Py_INCREF(gom);
	    PyObject_SetAttrString(main_module_, "gom", gom);
	} else {
	    main_module_ = PyImport_AddModule("__main__");
	    PyObject* gom = PyGraphiteObject_New(this, false);
	    Py_INCREF(gom);
	    PyObject_SetAttrString(main_module_, "gom", gom);
	}

	//   If Python interpreter is embedded in Graphite,
	// redirect output and error to Graphite console.
	if(use_embedded_interpreter_) {
	    PyRun_SimpleString(
		"class OutGraphiteStream:                   \n"
		"  def __init__(self):                      \n"
		"     pass                                  \n"
		"  def write(self, string):                 \n"
		"     if string != \'\\n\':                 \n"
		"        gom.out(string)                    \n"
		"  def flush(self):                         \n"
		"     pass                                  \n"
		"class ErrGraphiteStream:                   \n"
		"  def __init__(self):                      \n"
		"     self.buffer = str()                   \n"
		"  def write(self, string):                 \n"
		"     for c in string:                      \n"
		"        self.putc(c)                       \n"
		"  def putc(self,c):                        \n"
		"     if c == '\\\n':                       \n"
		"        self.flush()                       \n"
		"     else:                                 \n"
		"        self.buffer += c                   \n"
		"  def flush(self):                         \n"
		"     gom.err(self.buffer)                  \n"
		"     self.buffer = str()                   \n"
		"                                           \n"
		"import sys                                 \n"
		"sys.stdout = OutGraphiteStream()           \n"
		"sys.stderr = ErrGraphiteStream()           \n"
		"sys.displayhook = gom.out                  \n"
	    );
	}
	Process::enable_FPE(FPE_bkp);
    }

    PythonInterpreter::~PythonInterpreter() {
	if(use_embedded_interpreter_) {
	    Py_Finalize();
	}
	main_module_ = nullptr;
    }

    void PythonInterpreter::reset() {
	if(use_embedded_interpreter_) {
	    Py_Finalize();
	}
	main_module_ = nullptr;
	// TODO: restart
    }

    bool PythonInterpreter::execute(
	const std::string& command, bool save_in_history, bool log
    ) {
        if(log) {
            Logger::out("GOMpy") << command << std::endl;
        }

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);
        int res = PyRun_SimpleString(const_cast<char*>(command.c_str()));
	Process::enable_FPE(FPE_bkp);

        if(res == -1) {
            return false;
        }
        if(save_in_history) {
            add_to_history(command);
        }

        return true;
    }

    bool PythonInterpreter::execute_file(const std::string& file_name) {

        Environment::instance()->set_value("current_gel_file", file_name);

// We got some problems under Windows, so we use this quick and dirty
// workaround...
#ifdef GEO_OS_WINDOWS
        std::string gel_file(file_name);
        if(!FileManager::instance()->find_file(gel_file)) {
            Logger::err("GOMpy") << "Cannot find file \'"
                               << gel_file << "\'" << std::endl;
            return false;
        }
        std::ifstream in(gel_file.c_str());
        std::stringstream file_buff;
        if(!in) {
            Logger::err("GOMpy") << "cannot open file:" << gel_file
                               << std::endl;
            return false;
        }
        while(in) {
	    std::string buff;
	    std::getline(in,buff);
            file_buff << buff << std::endl;
        }

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);
        int res = PyRun_SimpleString(file_buff.str().c_str());
	Process::enable_FPE(FPE_bkp);

        if(res == -1){
            return false;
        }
#else
        std::string gel_file(file_name);
        if(!FileManager::instance()->find_file(gel_file)) {
            Logger::err("GOMpy") << "Cannot find file \'"
                               << gel_file << "\'" << std::endl;
            return false;
        }
        FILE* f = fopen(gel_file.c_str(), "rt");
        if(f == nullptr) {
            Logger::err("GOMpy") << "Cannot open file:" << gel_file
                                       << std::endl;
            return false;
        }

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);
        int res = PyRun_SimpleFile(f, const_cast<char*>(gel_file.c_str()));
	Process::enable_FPE(FPE_bkp);

        fclose(f);
        if(res == -1){
            return false;
        }
#endif
        return true;
    }

    void PythonInterpreter::bind(const std::string& id, const Any& value) {
	PyObject* obj = graphite_to_python(value);
	Py_INCREF(obj);
        PyObject_SetAttrString(main_module_, id.c_str(), obj);
    }

    Any PythonInterpreter::resolve(
	const std::string& id, bool quiet
    ) const {
	Any any_result;
	PyObject* result =
	    PyObject_GetAttrString(main_module_, id.c_str());
	PyErr_Clear();
	if(result == nullptr) {
	    if(!quiet) {
		Logger::err("GOMpy") << id << ":no such global object"
				     << std::endl;
	    }
	    return any_result;
	}
	any_result = python_to_graphite(result);
	return any_result;
    }

    Any PythonInterpreter::eval(
	const std::string& expression, bool quiet
    ) const {
	// return resolve(expression, quiet);
	Any any_result;
	PyCodeObject* code = (PyCodeObject*) Py_CompileString(
	    expression.c_str(), "immediate", Py_eval_input
	);
	PyErr_Clear();
	if(code == nullptr) {
	    if(!quiet) {
		Logger::err("GOMpy") << expression << ":could not interpret"
				     << std::endl;
	    }
	    return any_result;
	}

	PyObject* global_dict = PyModule_GetDict(main_module_);
	Py_INCREF(global_dict);
	PyObject* local_dict = PyDict_New();
	Py_INCREF(local_dict);
	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);
	PyObject* result = PyEval_EvalCode(
	    (PyObject*)code, global_dict, local_dict
	);
	Process::enable_FPE(FPE_bkp);
	Py_XINCREF(result);
	any_result = python_to_graphite(result);
	Py_XDECREF(result);
	Py_DECREF(local_dict);
	Py_DECREF(global_dict);

	return any_result;
    }

    void PythonInterpreter::display_error_message(const std::string& msg) {
	Logger::err("Python") << msg << std::endl;
    }

    void PythonInterpreter::list_names(std::vector<std::string>& names) const {
	names.clear();
	PyObject* globals = PyModule_GetDict(main_module_);
	Py_ssize_t nb = PyDict_Size(globals);
	PyObject* keys = PyDict_Keys(globals);
	Py_INCREF(keys);
	for(int i=0; i<nb; ++i) {
	    names.push_back(python_to_string(PyList_GetItem(keys,i)));
	}
	Py_DECREF(keys);
    }

    /*****************************************************************/

    void PythonInterpreter::get_keys(
	const std::string& context, std::vector<std::string>& keys
    ) {
	return Interpreter::get_keys(context, keys);
    }


}

/**
 * \brief Initializing function dlsym-ed by the Python interpreter
 *  when Graphite is loaded in an existing Python interpreter (i.e.,
 *  not using the main Graphite application.
 */
extern "C" gompy_API PyObject* PyInit_libgompy(void);
extern "C" gompy_API PyObject* PyInit_libgompy() {
    CmdLine::declare_arg("gel", "Python",
	    "Name of the graphite embedded language runtime"
    );
    CmdLine::set_arg("log:pretty",false);
    PyObject* result = PyInit_gom();
    Interpreter::initialize(new PythonInterpreter,"Python","py");
    Interpreter::initialize(new LuaInterpreter, "Lua", "lua");
    return result;
}

/**
 * \brief An alias for PyInit_libgompy(), because under Windows
 *  DLL names are not prefixed by "lib".
 */
extern "C" gompy_API PyObject* PyInit_gompy(void);
extern "C" gompy_API PyObject* PyInit_gompy() {
    return PyInit_libgompy();
}
