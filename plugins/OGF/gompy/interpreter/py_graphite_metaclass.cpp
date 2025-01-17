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

namespace OGF {
    namespace GOMPY {

	/**
	 * \brief Class definition for Python wrapper
	 *  around Graphite MetaClass objects.
	 */
	PyTypeObject graphite_MetaClassType = {
	    PyVarObject_HEAD_INIT(nullptr, 0)
	    "graphite.MetaClass",     // tp_name
	    sizeof(graphite_Object)   // tp_basicsize
	    // The rest is initialized by init_graphite_MetaClassType()
	};

	/**
	 * \brief Function to initialize graphite_MetaClassType
	 */
	void init_graphite_MetaClassType() {
	    graphite_MetaClassType.tp_call       = graphite_call;
	    graphite_MetaClassType.tp_dealloc    = graphite_Object_dealloc;

	    // TYPE_SUBCLASS so that Python knows it is a type, and
	    // HEAPTYPE, so that Python knows it is not a static type
	    graphite_MetaClassType.tp_flags      = Py_TPFLAGS_DEFAULT |
		                                   Py_TPFLAGS_TYPE_SUBCLASS |
		                                   Py_TPFLAGS_HEAPTYPE ;

	    graphite_MetaClassType.tp_getset     = graphite_Object_getsets;
	    graphite_MetaClassType.tp_base       = &graphite_ObjectType;
	    graphite_MetaClassType.tp_new        = graphite_Object_new;

	    // Note: not using Py_TPFLAGS_MANAGED_WEAKREF, but we
	    // need tp_weaklistoffset to be set else it crashes
	    // disclaimer: I do not understand what weakrefs are !!
	    graphite_MetaClassType.tp_weaklistoffset = offsetof(
		graphite_Object, weakrefs
	    );

	}
    }
}
