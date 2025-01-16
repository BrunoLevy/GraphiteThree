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

#include <OGF/gompy/interpreter/nl_vector_interop.h>
#include <OGF/scene_graph/NL/vector.h>
#include <OGF/gom/reflection/meta.h>


/************** NumPy Interop **********************************/

/***** NumPy defines *****/

typedef struct {
    int two;         // sanity check, should contain the integer value 2
    int nd;          // number of dimensions
    char typekind;   // i: integer, b: boolean, u: unsigned integer, f: float
                     // d: double
    int itemsize;    // size of each element
    int flags;       // See belox
    Py_intptr_t* shape;   // dimensions (length = nd)
    Py_intptr_t* strides; // strides (length = nd)
    void *data;      // Pointer to first item
    PyObject *descr; // Optional descriptor
} PyArrayInterface;

#define NPY_ARRAY_C_CONTIGUOUS    0x0001
#define NPY_ARRAY_ALIGNED         0x0100
#define NPY_ARRAY_NOTSWAPPED      0x0200
#define NPY_ARRAY_WRITEABLE       0x0400

/*
#define NPY_ARRAY_F_CONTIGUOUS    0x0002
#define NPY_ARRAY_OWNDATA         0x0004
#define NPY_ARRAY_FORCECAST       0x0010
#define NPY_ARRAY_ENSURECOPY      0x0020
#define NPY_ARRAY_ENSUREARRAY     0x0040
#define NPY_ARRAY_ELEMENTSTRIDES  0x0080
#define NPY_ARRAY_HAS_DESCR       0x0800
#define NPY_ARRAY_UPDATEIFCOPY    0x1000 // Deprecated in 1.14
#define NPY_ARRAY_WRITEBACKIFCOPY 0x2000
*/

namespace {
    using namespace OGF;

    /**
     * \brief Deletes a PyArrayInterface.
     * \param[in] array_interface a pointer to
     *  the PyArrayInterface to be deleted.
     * \details Used for NumPy interop.
     */
    void delete_array_interface_internal(
	PyArrayInterface* array_interface
    ) {
	delete[] array_interface->shape;
	delete[] array_interface->strides;
	delete array_interface->descr;
	delete array_interface;
    }

}


namespace OGF {

    bool python_to_nl_vector(PyObject* obj, Any& result, MetaType* mtype) {
	bool conversion_OK = false;
	if(mtype != ogf_meta<::OGF::NL::Vector*>::type()) {
	    return false;
	}

        if(PyObject_HasAttrString(obj,"__array_struct__")) {
            PyObject* capsule = PyObject_GetAttrString(obj,"__array_struct__");
            Py_INCREF(capsule);
            if(PyCapsule_CheckExact(capsule)) {
                void* ptr = PyCapsule_GetPointer(capsule,nullptr);
                geo_assert(ptr != nullptr);

                PyArrayInterface* array_interface =
                    static_cast<PyArrayInterface*>(ptr);

                if(array_interface != nullptr) {
		    // sanity check
		    geo_assert(array_interface->two == 2);

		    char typekind = array_interface->typekind;


		    // Workaround. I do not understand why, but when I create
		    // a numpyarray with datatype 'double', it appears here
		    // as 'f', so I check itemsize to detect doubles.
		    // TODO: understand what's going on here.
		    if(typekind == 'f' && array_interface->itemsize == 8) {
			typekind = 'd';
		    }

		    MetaType* element_meta_type = nullptr;
		    switch(typekind) {
		    case 'f':
			element_meta_type = ogf_meta<float>::type();
			break;
		    case 'd':
			element_meta_type = ogf_meta<double>::type();
			break;
		    case 'i':
			element_meta_type =
			    ogf_meta<Numeric::int32>::type();
			break;
		    case 'u':
			element_meta_type =
			    ogf_meta<Numeric::uint32>::type();
			break;
		    case 'b':
			element_meta_type =
			    ogf_meta<Numeric::uint8>::type();
			break;
		    }

		    if(
			array_interface->nd <= 2 &&
			element_meta_type != nullptr &&
			(array_interface->flags & NPY_ARRAY_C_CONTIGUOUS) != 0
		    ) {
			index_t dim = 1;
			if(array_interface->nd == 2) {
			    dim = index_t(array_interface->shape[1]);
			}
			SmartPointer<Counted> graphite_vector = new NL::Vector(
			    nullptr,
			    array_interface->data,
			    index_t(array_interface->shape[0]),
			    dim,
			    element_meta_type,
			    ((array_interface->flags & NPY_ARRAY_WRITEABLE) == 0)
			);
			result.set_value(graphite_vector);
			conversion_OK = true;
		    }
                }
            }
	    if(!conversion_OK) {
		// There was an array-like object, but it could not be accessed
		// as a NL::Vector
		Logger::err("gompy") << "Could not access array as NL::Vector"
				     << std::endl;
		// We return a null smart pointer
		SmartPointer<Counted> graphite_vector;
		result.set_value(graphite_vector);
		conversion_OK = true;
	    }
            Py_DECREF(capsule);
	}

	return conversion_OK;
    }

    /**************************************************************************/

    void delete_array_interface(PyObject* capsule) {
        void* ptr = PyCapsule_GetPointer(capsule,nullptr);
        geo_assert(ptr != nullptr);
	PyArrayInterface* array_interface = static_cast<PyArrayInterface*>(ptr);
	delete_array_interface_internal(array_interface);
    }

    /**************************************************************************/

    PyObject* create_array_interface(NL::Vector* vector) {

	PyArrayInterface* array_interface = new PyArrayInterface;

	array_interface->two = 2;

	array_interface->flags =
	    NPY_ARRAY_C_CONTIGUOUS |
	    NPY_ARRAY_ALIGNED |
	    NPY_ARRAY_WRITEABLE |
	    NPY_ARRAY_NOTSWAPPED ;

	if(vector->dimension() == 1) {
	    array_interface->nd    = 1;
	    array_interface->shape = new Py_intptr_t[1];
	    array_interface->shape[0] = Py_intptr_t(vector->get_nb_elements());
	} else {
	    array_interface->nd    = 2;
	    array_interface->shape = new Py_intptr_t[2];
	    array_interface->shape[0] = Py_intptr_t(vector->get_size());
	    array_interface->shape[1] = Py_intptr_t(vector->dimension());
	}

	array_interface->data = vector->data();
	array_interface->strides = nullptr;
	array_interface->descr = nullptr;

	MetaType* type = vector->get_element_meta_type();
	array_interface->typekind = '\0';
	array_interface->itemsize = 0;

	if(type->life_cycle() != nullptr) {
	    array_interface->itemsize =
		int(type->life_cycle()->object_size());
	}

	if(
	    type == ogf_meta<int>::type() ||
	    type == ogf_meta<signed_index_t>::type()
	) {
	    array_interface->typekind = 'i';
	} else if(
	    type == ogf_meta<unsigned int>::type() ||
	    type == ogf_meta<index_t>::type()
	) {
	    array_interface->typekind = 'u';
	} else if(
	    type == ogf_meta<float>::type() ||
	    type == ogf_meta<double>::type()
	) {
	    array_interface->typekind = 'f';
	} else if(type == ogf_meta<bool>::type()) {
	    array_interface->typekind = 'b';
	    array_interface->itemsize = 1;
	}

	if(
	    array_interface->typekind == '\0' ||
	    array_interface->itemsize == 0
	) {
	    Logger::err("GOMPy") << "Could not determine element type"
				 << std::endl;
	    delete_array_interface_internal(array_interface);
	    Py_INCREF(Py_None);
	    return Py_None;
	}

	return PyCapsule_New(
	    array_interface, nullptr, delete_array_interface
	);
    }


}
