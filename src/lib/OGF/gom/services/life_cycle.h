/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
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

#ifndef H_OGF_BASIC_ERTTI_SERIALIZER_H
#define H_OGF_BASIC_ERTTI_SERIALIZER_H

#include <OGF/gom/common/common.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/counted.h>

/**
 * \file OGF/gom/services/serializer.h
 * \brief Implementation of generic object lifecycle service
 */

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

namespace OGF {

    /**
     * \brief Manages the life cycle of an object.
     */
    class GOM_API LifeCycle : public Counted {
      public:

	/**
	 * \brief LifeCycle constructor.
	 * \param[in] object_size the size of an object in bytes.
	 */
        LifeCycle(size_t object_size, bool is_pod=false) :
	   object_size_(object_size), is_pod_(is_pod) {
	}

	/**
	 * \brief LifeCycle destructor.
	 */
	virtual ~LifeCycle();

	/**
	 * \brief Gets the size of an object.
	 * \return the size of an object in bytes.
	 */
	size_t object_size() const {
	    return object_size_;
	}

	/**
	 * \brief Tests whether object is pod (plain ordinary
	 *  datatype).
	 * \details Plain ordinary datatypes can be copied with
	 *  memcpy().
	 * \retval true if object is pod.
	 * \retval false otherwise.
	 */
	bool is_pod() const {
	    return is_pod_;
	}
	
	/**
	 * \brief Calls the constructor of an object.
	 * \param[in] address the address of the object
	 *  to be constructed.
	 * \details No memory allocation is done.
	 */
	virtual void construct(Memory::pointer address) = 0;

	/**
	 * \brief Copy-Constructs an object at a given address.
	 * \param[in] lhs the address where the object should
	 *  be constructed.
	 * \param[in] rhs the address of the right-hand-side object
	 *  to be copied.
	 * \details No memory allocation is done.
	 */
	virtual void copy_construct(Memory::pointer lhs, Memory::pointer rhs) = 0;

	/**
	 * \brief Calls the destructor of an object.
	 * \param[in] address the address of the object to be destructed.
	 * \details No memory deallocation is done.
	 */
	virtual void destroy(Memory::pointer address) = 0;

	/**
	 * \brief Calls the assignment operator.
	 * \param[in] lhs the address of the left hand side.
	 * \param[in] rhs the address of the right hand side.
	 */
	virtual void assign(Memory::pointer lhs, Memory::pointer rhs) = 0;	

	/**
	 * \brief Calls the constructor of objects in an array.
	 * \param[in] address the address of the object array
	 *  to be constructed.
	 * \param[in] nb number of objects.
	 * \details No memory allocation is done.
	 */
	virtual void construct_array(Memory::pointer address, index_t nb) = 0;

	/**
	 * \brief Copy-Constructs an array of object at a given address.
	 * \param[in] lhs the address of the objects to be constructed.
	 * \param[in] rhs the address of the right-hand-side objects
	 *  to be copied.
	 * \param[in] nb number of objects
	 * \details No memory allocation is done.
	 */
	virtual void copy_construct_array(Memory::pointer lhs, Memory::pointer rhs, index_t nb) = 0;

	/**
	 * \brief Destroys an array of object at a given address.
	 * \param[in] address the address of the objects to be destroyed.
	 * \param[in] nb number of objects
	 * \details No memory deallocation is done.
	 */
	virtual void destroy_array(Memory::pointer address, index_t nb) = 0;

	/**
	 * \brief Copies an array of object at a given address.
	 * \param[in] lhs the address of the left-hand-side objects
	 * \param[in] rhs the address of the right-hand-side objects
	 *  to be copied.
	 * \param[in] nb number of objects
	 * \details No memory allocation is done.
	 */
	virtual void assign_array(Memory::pointer lhs, Memory::pointer rhs, index_t nb) = 0;	

	/**
	 * \brief Dynamically allocates a new object.
	 * \return the address of the new object.
	 */
	virtual Memory::pointer new_object() = 0;

	/**
	 * \brief Dynamically allocates a new object with copy constructor.
	 * \param[in] rhs the address of the right-hand-side objects
	 *  to be copied.
	 * \return the address of the new object.
	 */
	virtual Memory::pointer new_object(Memory::pointer rhs) = 0;
	
	/**
	 * \brief Deletes an object.
	 * \param[in] address the address of the object.
	 */
	virtual void delete_object(Memory::pointer address) = 0;

	/**
	 * \brief Dynamically allocates an array of objects.
	 * \param[in] nb number of objects.
	 */
	virtual Memory::pointer new_array(index_t nb) = 0;

	/**
	 * \brief Deletes an array of objects.
	 * \param[in] address the address of the object array to 
	 *  be deleted.
	 */
	virtual void delete_array(Memory::pointer address) = 0;
	
      private:
	size_t object_size_;
	bool is_pod_;
    };

    /**
     * \brief A reference_counted pointer to a LifeCycle.
     */
    typedef SmartPointer<LifeCycle> LifeCycle_var;

    /*************************************************************************/
    
    /**
     * \brief Concrete implementation of LifeCycle.
     */
    template <class T> class GenericLifeCycle : public LifeCycle {
      public:

	/**
	 * \brief GenericLifeCycle constructor.
	 */
        GenericLifeCycle() : LifeCycle(sizeof(T), std::is_pod<T>::value) {
	}

	/**
	 * \copydoc LifeCycle::construct()
	 */
	void construct(Memory::pointer address) override {
	    new(address)T;
	}

	/**
	 * \copydoc LifeCycle::copy_construct()
	 */
	void copy_construct(Memory::pointer lhs, Memory::pointer rhs) override {
	    new(lhs)T(*(T*)rhs);
	}
	
	/**
	 * \copydoc LifeCycle::destroy()
	 */
	void destroy(Memory::pointer address) override {
	    geo_argused(address); // Silences a MSVC warning.
	    ((T*)address)->~T();
	}

	/**
	 * \copydoc LifeCycle::assign()
	 */
	void assign(Memory::pointer lhs, Memory::pointer rhs) override {
	    *(T*)lhs=*(T*)rhs;
	}	

	/**
	 * \copydoc LifeCycle::construct_array()
	 */
	void construct_array(Memory::pointer address, index_t nb) override {
	    T* array = (T*)address;
	    for(index_t i=0; i<nb; ++i) {
		new(&array[i])T;
	    }
	}

	/**
	 * \copydoc LifeCycle::copy_construct_array()
	 */
	void copy_construct_array(
	    Memory::pointer lhs, Memory::pointer rhs, index_t nb
	) override {
	    T* lhs_array = (T*)lhs;
	    T* rhs_array = (T*)rhs;	    
	    for(index_t i=0; i<nb; ++i) {
		new(&lhs_array[i])T(rhs_array[i]);
	    }
	}

	/**
	 * \copydoc LifeCycle::destroy_array()
	 */
	void destroy_array(Memory::pointer address, index_t nb) override {
	    T* array = (T*)address;
	    geo_argused(array); // Silences a MSVC warning.
	    for(index_t i=0; i<nb; ++i) {
		array[i].~T();
	    }
	}

	/**
	 * \copydoc LifeCycle::assign_array()
	 */
	void assign_array(
	    Memory::pointer lhs, Memory::pointer rhs, index_t nb
	) override {
	    T* lhs_array = (T*)lhs;
	    T* rhs_array = (T*)rhs;	    
	    for(index_t i=0; i<nb; ++i) {
		lhs_array[i] = rhs_array[i];
	    }
	}	

	/**
	 * \copydoc LifeCycle::new_object()
	 */
	Memory::pointer new_object() override {
	    return Memory::pointer(new T);
	}

	/**
	 * \copydoc LifeCycle::new_object()
	 */
	Memory::pointer new_object(Memory::pointer rhs) override {
	    return Memory::pointer(new T(*(T*)rhs));
	}
	
	/**
	 * \copydoc LifeCycle::delete_object()
	 */
	void delete_object(Memory::pointer address) override {
	    delete((T*)address);
	}

	/**
	 * \copydoc LifeCycle::new_array()
	 */
	Memory::pointer new_array(index_t nb) override {
	    return Memory::pointer(new T[nb]);	    
	}

	/**
	 * \copydoc LifeCycle::delete_array()
	 */
	void delete_array(Memory::pointer address) override {
	    delete[]((T*)address);
	}
	
    };
    
}

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic pop
#endif

#endif
